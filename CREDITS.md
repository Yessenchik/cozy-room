# Credits

## Team

- Yessen: project setup, window, shaders, camera, lighting, empty room
- Aidana: bed, bookshelf, rug, plant, textures
- Aruzhan: desk, chair, desk lamp and its light, scene graph
- Baxa: day/night cycle, time controls, on-screen help, sun shadows, final checks

## Libraries

| Library | Used for | License |
|---------|----------|---------|
| [GLFW 3.4](https://www.glfw.org/) | window, OpenGL context, input | zlib/libpng |
| [GLM 1.0.1](https://github.com/g-truc/glm) | vectors and matrices | MIT (Happy Bunny License also offered) |
| [GLAD](https://github.com/Dav1dde/glad) | loading OpenGL 3.3 core functions | generator MIT; generated code public domain / CC0, `khrplatform.h` (c) The Khronos Group, MIT-style |
| [stb_easy_font](https://github.com/nothings/stb) by Sean Barrett | on-screen text | public domain / MIT |

GLFW and GLM are downloaded by CMake (FetchContent); GLAD and stb_easy_font are kept in `external/`.

## References

- Joey de Vries, *LearnOpenGL*: "Basic Lighting", "Advanced Lighting" (Blinn-Phong)
  and "Shadow Mapping" chapters, https://learnopengl.com (CC BY-NC 4.0).
  Used as a reference for the lighting model, shadow mapping, PCF and shadow bias.
  No code was copied as-is.
- Course materials of Computer Graphics Fundamentals, Astana IT University.

## Assets

All models are built from scaled cubes in code; no 3D models are downloaded.
Textures: (to be added together with the textures task).

## AI tools

Claude (Anthropic) was used as an assistant; see the "AI Use" section in the README.
