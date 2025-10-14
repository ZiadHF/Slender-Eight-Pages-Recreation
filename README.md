# Slender: The Eight Pages - OpenGL Recreation

This repository contains a fan-made OpenGL recreation of the classic indie horror experience "Slender: The Eight Pages." The goal of the project is to implement the core gameplay loop, exploring a dark forest, finding eight pages, and surviving encounters with the stalking entity using OpenGL.

IMPORTANT: This is an unofficial, fan-made project and is not affiliated with or endorsed by the original creators of Slender: The Eight Pages.

## Highlights

- Lightweight OpenGL-based renderer using GLFW, GLAD, GLM and custom shaders.
- Config-driven levels and assets in `config/` and `assets/`.
- Small, readable codebase intended for teaching Graphics/Engine concepts.
- Postprocessing and basic audio.

## Gameplay (Short)

You explore a dimly-lit forest searching for eight pages scattered across the map. A mysterious, unkillable entity stalks you, look away or stay in the open too long and it will end the game. The objective is to collect all eight pages and reach the exit or survive until all pages are found.

This recreation focuses on core mechanics: exploration, page collection, simple AI/encounters, and atmospheric rendering.

## Screenshots (TBA)

## Controls

For custom mappings, see `config/player.json` for controls.

- Move: W A S D
- Sprint / Walk modifier: Left Shift
- Look: mouse (free look)
- Toggle flashlight: F
- Interact / Pick up page: Left Click (when near a page)
- Pause / Menu: Esc

## Quick Build & Run (Windows)

### Run (if already built)

```powershell
cd bin
Slender.exe
```

## Build & Run in VS Code

1. **Install:**

   - [CMake](https://cmake.org/download/)
   - [Visual Studio Build Tools](https://visualstudio.microsoft.com/downloads/)
   - VS Code extensions: _CMake Tools_ & _C/C++_

2. **Open Project:**
   Open the folder with `CMakeLists.txt` in VS Code.

3. **Configure & Build:**

   - `Ctrl+Shift+P` → **CMake: Configure**
   - `Ctrl+Shift+P` → **CMake: Build**

4. **Run the Game:**

   ```powershell
   cd bin
   .\Slender.exe
   ```

## Project layout

- `source/` — application and engine source code; main entry is `main.cpp`.
- `assets/` — models, textures, shaders and other runtime assets.
- `assets/shaders/` — GLSL shaders used by the renderer.
- `config/` — JSONC level, entity and material test files used to compose scenes.
- `build/` — CMake intermediate files and build output (ignored by VCS normally).
- `bin/` — typical runtime output location for the compiled executable.
- `vendor/` — included 3rd-party libraries (GLFW, GLAD, GLM, ImGui, etc.).
