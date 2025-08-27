# Eclipsera Engine
Eclipsera Engine is a modified version of LunarEngine, designed to include features that the original project currently lacks. This engine is built using ROBLOX LuaU and aims to support the full range of Roblox APIs for game development.

The project is still in its early stages and actively under development.
## Is Eclipsera Engine Open Source?
Yes. Since this is a modification of the open-source LunarEngine, Eclipsera Engine is also open source and freely available for everyone to use.
## Current Features
- Separated shader files: `shaders/shaders.h` and `shaders/sky.h` have been split from `Renderer.cpp` for better organization.
- Basic support for the Roblox API, including `UserInputService`
> (that only service that LunarEngine doesn't have it rn, be patience, i will add more but might take a time, i'm no expert at cpp language lol)
- Early-stage support for GLSL shaders.
- Performance improvements, Fixed a crash issue in the Lua scheduler that occurred when loading large LuaU files.
> (Note: creating over 100,000 instances at once still causes crashes. this is currently a work in progress.)
## Roadmap
- Add a new instance/class called `Skybox`.
- Provide full GLSL shader support, along with legacy/basic post-processing options that can be used in the `LightingService` without requiring prior knowledge of GLSL.
- Implement an ImGUI-based debugging tool to support development. This will include access to an Explorer panel, Properties, memory usage stats, and hardware information.
- Rewrite LunarEngine’s ambient occlusion system using a modern technique, with customizable settings available through the `LightingService`.
- Improve camera controls: transition from keyboard arrow-based control to a more standard right mouse button handler.
