# Cozy Room

A real-time 3D diorama of a small bedroom, written in C++ with OpenGL 3.3 core.
Final project for Computer Graphics Fundamentals, Astana IT University.

Stretch goal: day/night cycle with a moving sun.

## Team

| Member  | Part |
|---------|------|
| Yessen  | Project setup, window, shaders, camera, lighting, empty room |
| Aidana  | Bed, bookshelf, rug, plant, textures |
| Aruzhan | Desk, chair, desk lamp and its light, scene graph |
| Baxa    | Day/night cycle, time controls, help screen, final checks |

## Build

Requirements: CMake 3.16+, a C++17 compiler, internet on the first build
(GLFW and GLM are downloaded by CMake). GLAD is included in `external/`.

```bash
cmake -S . -B build
cmake --build build
./build/CozyRoom            # Windows: build\Debug\CozyRoom.exe
```

VS Code: open the folder with the CMake Tools extension, then Build and Run.

Xcode:

```bash
cmake -S . -B build-xcode -G Xcode
open build-xcode/CozyRoom.xcodeproj
```

## Controls

| Input | Action |
|-------|--------|
| Right mouse drag | Orbit around the room |
| Scroll | Zoom |
| A / D | Orbit left / right |
| Q / E | Lower / raise the camera |
| W / S | Zoom in / out |
| Esc | Quit |

The FPS counter is shown in the window title.

## Features

- [x] OpenGL 3.3 core profile, VAO/VBO/EBO, own GLSL shaders
- [x] Orbit camera, frame-rate independent
- [x] Per-fragment Blinn-Phong lighting with a directional sun
- [x] Room: floor, two walls, window
- [x] FPS counter
- [ ] Furniture
- [ ] Textures with mipmaps
- [ ] Desk lamp (second light, toggle)
- [ ] Day/night cycle

## AI Use

To be completed: which AI tools were used, for what, and what we changed.
