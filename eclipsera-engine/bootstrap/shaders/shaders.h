#pragma once

// ---------------- GLSL Version ----------------
#define GLSL_VERSION 330

// Lit + CSM shadows (PCF), hemispheric, spec, fresnel, AO
inline const char* LIT_VS = R"(

#version 330
in vec3 vertexPosition;
in vec3 vertexNormal;

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 lightVP0;
uniform mat4 lightVP1;
uniform mat4 lightVP2;

uniform float normalBiasWS0;
uniform float normalBiasWS1;
uniform float normalBiasWS2;

out vec3 vN;
out vec3 vWPos;
out vec4 vLS0;
out vec4 vLS1;
out vec4 vLS2;

void main(){
    mat3 nmat = mat3(transpose(inverse(matModel)));
    vN = normalize(nmat * vertexNormal);
    vec4 worldPos = matModel * vec4(vertexPosition,1.0);
    vWPos = worldPos.xyz;

    // apply normal-space small offset per-cascade to avoid contact gaps
    vec3 Nw = normalize(vN);
    vec4 worldPos0 = worldPos + vec4(Nw * normalBiasWS0, 0.0);
    vec4 worldPos1 = worldPos + vec4(Nw * normalBiasWS1, 0.0);
    vec4 worldPos2 = worldPos + vec4(Nw * normalBiasWS2, 0.0);

    vLS0 = lightVP0 * worldPos0;
    vLS1 = lightVP1 * worldPos1;
    vLS2 = lightVP2 * worldPos2;

    gl_Position = mvp * vec4(vertexPosition,1.0);
}

)";

inline const char* LIT_VS_INST = R"(

#version 330
in vec3 vertexPosition;
in vec3 vertexNormal;

// Per-instance model matrix provided as vertex attribute
in mat4 instanceTransform;

uniform mat4 mvp;
uniform mat4 lightVP0;
uniform mat4 lightVP1;
uniform mat4 lightVP2;

uniform float normalBiasWS0;
uniform float normalBiasWS1;
uniform float normalBiasWS2;

out vec3 vN;
out vec3 vWPos;
out vec4 vLS0;
out vec4 vLS1;
out vec4 vLS2;

void main(){
    mat3 nmat = mat3(transpose(inverse(instanceTransform)));
    vN = normalize(nmat * vertexNormal);

    vec4 worldPos = instanceTransform * vec4(vertexPosition,1.0);
    vWPos = worldPos.xyz;

    vec3 Nw = normalize(vN);
    vec4 worldPos0 = worldPos + vec4(Nw * normalBiasWS0, 0.0);
    vec4 worldPos1 = worldPos + vec4(Nw * normalBiasWS1, 0.0);
    vec4 worldPos2 = worldPos + vec4(Nw * normalBiasWS2, 0.0);

    vLS0 = lightVP0 * worldPos0;
    vLS1 = lightVP1 * worldPos1;
    vLS2 = lightVP2 * worldPos2;

    gl_Position = mvp * worldPos;
}
    
)";

inline const char* LIT_FS = R"(

#version 330
in vec3 vN;
in vec3 vWPos;
in vec4 vLS0;
in vec4 vLS1;
in vec4 vLS2;

out vec4 FragColor;

uniform vec3 viewPos;

// Lighting
uniform vec3 sunDir;
uniform vec3 skyColor;
uniform vec3 groundColor;
uniform float hemiStrength;
uniform float sunStrength;
uniform vec3 ambientColor;    // flat ambient (default 0)

// Spec/Fresnel
uniform float specStrength;   // 0..1 intensity
uniform float shininess;      // 8..128
uniform float fresnelStrength;// 0..1

// AO
uniform float aoStrength;     // 0..1
uniform float groundY;        // ground plane Y

// CSM
uniform sampler2DShadow shadowMap0;
uniform sampler2DShadow shadowMap1;
uniform sampler2DShadow shadowMap2;
uniform int   shadowMapResolution;
uniform float pcfStep;        // in texels
uniform float biasMin;        // min bias
uniform float biasMax;        // max bias
uniform vec3  cascadeSplits;  // [d0,d1,f]

// transition band (fraction of split distance)
uniform float transitionFrac; // e.g. 0.10..0.20 of split distance

// exposure
uniform float exposure;       // new exposure control

// raylib default material color (tint * material diffuse)
uniform vec4 colDiffuse;

float SampleShadow(sampler2DShadow smap, vec3 proj, float ndl){
    float bias = mix(biasMax, biasMin, ndl);
    float step = pcfStep / float(shadowMapResolution);
    float sum = 0.0;
    for(int x=-1; x<=1; ++x)
    for(int y=-1; y<=1; ++y){
        vec2 off = vec2(x,y) * step;
        sum += texture(smap, vec3(proj.xy + off, proj.z - bias));
    }
    return sum / 9.0;
}

float ShadowForCascade(int idx, vec3 proj, float ndl){
    if (proj.x<0.0 || proj.x>1.0 || proj.y<0.0 || proj.y>1.0 || proj.z<0.0 || proj.z>1.0)
        return 1.0;
    if (idx==0) return SampleShadow(shadowMap0, proj, ndl);
    if (idx==1) return SampleShadow(shadowMap1, proj, ndl);
    return SampleShadow(shadowMap2, proj, ndl);
}

// blend across cascade boundaries using a symmetric transition band
float ShadowBlend(vec3 proj0, vec3 proj1, vec3 proj2, float ndl, float viewDepth, vec3 splits){
    // compute absolute band sizes in world units based on splits
    float d0 = splits.x;
    float d1 = splits.y;
    // clamp a reasonable fraction
    float frac = clamp(transitionFrac, 0.0, 0.5);
    float band0 = max(0.001, d0 * frac);
    float band1 = max(0.001, d1 * frac);

    // below first band -> full cascade0
    if (viewDepth <= d0 - band0) {
        return ShadowForCascade(0, proj0, ndl);
    }
    // blend between 0 and 1
    if (viewDepth < d0 + band0) {
        float t = smoothstep(d0 - band0, d0 + band0, viewDepth);
        float s0 = ShadowForCascade(0, proj0, ndl);
        float s1 = ShadowForCascade(1, proj1, ndl);
        return mix(s0, s1, t);
    }
    // middle region -> full cascade1
    if (viewDepth <= d1 - band1) {
        return ShadowForCascade(1, proj1, ndl);
    }
    // blend between 1 and 2
    if (viewDepth < d1 + band1) {
        float t = smoothstep(d1 - band1, d1 + band1, viewDepth);
        float s1 = ShadowForCascade(1, proj1, ndl);
        float s2 = ShadowForCascade(2, proj2, ndl);
        return mix(s1, s2, t);
    }
    // far -> cascade2
    return ShadowForCascade(2, proj2, ndl);
}

void main(){
    vec3 N = normalize(vN);
    vec3 L = normalize(-sunDir);
    vec3 V = normalize(viewPos - vWPos);
    vec3 H = normalize(L + V);

    // Hemispheric ambient
    float t = clamp(N.y*0.5 + 0.5, 0.0, 1.0);
    vec3 hemi = mix(groundColor, skyColor, t) * hemiStrength;

    // Diffuse term
    float ndl = max(dot(N,L), 0.0);

    // Specular
    float spec = pow(max(dot(N,H), 0.0), shininess) * specStrength * 0;

    // Fresnel (Schlick)
    float F0 = 0.02;
    float FH = pow(1.0 - max(dot(N,V),0.0), 5.0);
    float fresnel = mix(F0, 1.0, FH) * fresnelStrength;

    // AO
    float aoHem = mix(0.4, 1.0, t);
    float height = max(vWPos.y - groundY, 0.0);
    float aoGround = clamp(1.0 - exp(-height*0.25), 0.6, 1.0);
    float ao = mix(1.0, aoHem*aoGround, aoStrength);

    // view-space depth as distance camera -> worldPos (monotonic)
    float viewDepth = length(viewPos - vWPos);

    // compute all three projections once
    vec3 p0 = (vLS0.xyz / vLS0.w) * 0.5 + 0.5;
    vec3 p1 = (vLS1.xyz / vLS1.w) * 0.5 + 0.5;
    vec3 p2 = (vLS2.xyz / vLS2.w) * 0.5 + 0.5;

    // blended shadow across cascade boundaries
    float shadow = ShadowBlend(p0, p1, p2, ndl, viewDepth, cascadeSplits);

    // Convert material color from sRGB to linear before lighting
    vec3 base = pow(colDiffuse.rgb, vec3(2.2));

    // Diffuse sunlight contribution
    float sunTerm = sunStrength * ndl * shadow;

    // Block specular and fresnel inside shadow
    spec    *= shadow;
    fresnel *= shadow;

    // Final color
    vec3 color = base * (hemi*ao + sunTerm) + spec + fresnel;
    color += base * ambientColor; // flat ambient

    // apply exposure before gamma
    color *= exposure;

    // Gamma correct (linear -> sRGB)
    color = pow(max(color, vec3(0.0)), vec3(1.0/2.2));
    FragColor = vec4(color, colDiffuse.a);
}
    
)";
