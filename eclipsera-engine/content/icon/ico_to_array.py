from PIL import Image
import sys

def ico_to_32_array(ico_path, out_path, varname="icon_data"):
    img = Image.open(ico_path)

    # collect all sub-icons
    frames = []
    i = 0
    try:
        while True:
            img.seek(i)
            frames.append((img.size, i))
            i += 1
    except EOFError:
        pass

    # choose largest size divisible by 32
    candidates = [s for s in frames if s[0][0] % 32 == 0 and s[0][1] % 32 == 0]
    if not candidates:
        raise ValueError("No sub-icon divisible by 32 found")
    best_size, best_index = max(candidates, key=lambda x: x[0][0]*x[0][1])
    # load and convert
    img.seek(best_index)
    rgba = img.convert("RGBA")

    # downscale if not already 32x32
    if rgba.size != (32, 32):
        rgba = rgba.resize((32, 32), Image.LANCZOS)

    pixels = list(rgba.getdata())
    flat = []
    for r, g, b, a in pixels:
        flat.extend([r, g, b, a])

    with open(out_path, "w") as f:
        f.write(f"// Generated from {ico_path} -> 32x32\n")
        f.write(f"unsigned char {varname}[] = {{\n")
        for i, val in enumerate(flat):
            if i % 12 == 0:
                f.write("    ")
            f.write(f"{val}, ")
            if i % 12 == 11:
                f.write("\n")
        f.write("};\n")
        f.write(f"const int {varname}_width = 32;\n")
        f.write(f"const int {varname}_height = 32;\n")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python ico_to_array.py input.ico output.h")
    else:
        ico_to_32_array(sys.argv[1], sys.argv[2])
