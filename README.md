# Cozy Room

A real-time 3D diorama of a small bedroom that lives through a full day,
written in C++ with OpenGL 3.3 core.
Final project for Computer Graphics Fundamentals, Astana IT University
(topic 2: Cozy Room Diorama).

![Morning sun through the window](docs/screenshots/11-morning-sun-through-window.png)
![Night, the desk lamp is on](docs/screenshots/13-night-desk-lamp.png)

## Team

| Member  | Part |
|---------|------|
| Yessen  | Project setup, window, shaders, camera, lighting, empty room, desk lamp shadows (cube map), Saturn picture |
| Aidana  | Bed with headboard, bookshelf, rug, plant, `Texture` class, UVs and all textures |
| Aruzhan | Desk, chair, desk lamp and its light, scene graph (lamp arm, cat), laptop screen, procedural sky in the window |
| Baxa    | Day/night cycle, time controls, on-screen help, sun shadows, fairy lights, glowing lamp shade, release checks |

## Build

Requirements: CMake 3.16+, a C++17 compiler, internet on the first build
(GLFW and GLM are downloaded by CMake). GLAD, stb_easy_font and stb_image are included in `external/`.

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

Linux: install the X11 development packages first
(`libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl-dev`).
If Wayland headers are missing, configure with `-DGLFW_BUILD_WAYLAND=OFF`.

## Controls

| Input | Action |
|-------|--------|
| Right mouse drag | Orbit around the room |
| Scroll, W / S | Zoom in / out |
| A / D | Orbit left / right |
| Q / E | Lower / raise the camera |
| Space | Pause / resume time |
| - / + | Time slower / faster (1 day = 6 s ... 8 min) |
| Left / Right (hold) | Scrub the time backwards / forwards |
| 1 / 2 / 3 / 4 | Jump to morning / noon / sunset / night |
| T | Time-lapse from 07:00 to 23:00 (the lamp switches on at 19:00) |
| Left mouse on the bottom bar | Drag the time slider |
| L | Desk lamp on / off |
| F | Fairy lights on / off |
| [ / ] | Turn the lamp arm |
| P | Laptop screen on / off |
| B | FPS benchmark (20 s) |
| V | Vsync on / off |
| H | Show / hide the help panel |
| Esc | Quit |

The clock, speed, lamp and fairy-light state and FPS are shown in the top-left corner;
the FPS is also in the window title.

## Features

Core (from the topic):

- [x] Room with furniture: bed, bookshelf with books, rug, plant, desk, chair, desk lamp
- [x] Sun light through the window, moving with a time slider
- [x] Desk lamp switched by a key (second light, point light with attenuation)
- [x] Fairy lights switched by a key (F): 12 small point lights over the bed, on by themselves at 19:00
- [x] Textured walls and floor (with mipmaps), plus a patterned rug and wood grain on the bed and bookshelf
- [x] Framed picture of Saturn above the bed (texture drawn by our own script)
- [x] A cat that moves around the room during the day, a laptop with a screen that turns on (P)
- [x] Procedural sky with the sun, moon and stars seen through the window

Stretch goals:

- [x] Day/night cycle: the sun moves along an arc, the moon lights the room at night
- [x] Warm/cold colour change by time of day (sky, sun and ambient colours)
- [x] Shadow mapping for the sun, soft edges with 3x3 PCF
- [x] Shadows from the desk lamp (cube-map shadow for a point light), e.g. under the desk
- [ ] Dust particles in the light beam

Wow moment: press **T** for a time-lapse from morning to night;
the sunlight patch crawls over the floor, and at 19:00 the desk lamp shade starts to glow
and the fairy lights come on one after another along the string.

Technical:

- [x] OpenGL 3.3 core profile, VAO/VBO/EBO, own GLSL shaders
- [x] Orbit camera, frame-rate independent
- [x] Per-fragment Blinn-Phong lighting
- [x] On-screen HUD with help (no font texture, text built from quads)
- [x] FPS counter and built-in benchmark
- [x] Scene graph (parent-child transforms): the lamp arm and shade follow the lamp base, the cat's parts follow its body
- [x] Emissive surfaces: the bulbs, the lamp shade when the lamp is on, the laptop screen

## How it works

**Day/night cycle** (`DayNight.cpp`). The clock runs from 0 to 24 hours. Between 06:00
and 18:00 the sun moves along an arc behind the back wall, rising on the left and setting
on the right; at night the moon follows the same arc. Sky, light and ambient colours
come from key frames (night, dawn, sunrise, morning, noon, afternoon, sunset, dusk) that
are blended smoothly. Near the horizon the light fades to zero, so the switch between
the sun and the moon is not visible. The lamp switches on at 19:00 and off at 07:00;
L still works at any time.

**Shadows** (`ShadowMap.cpp`, `shadow_depth.*`, `basic.frag`). Two passes per frame.
First the scene is drawn from the sun into a 2048x2048 depth texture with an orthographic
projection (the sun is far away, so its rays are parallel). Then `basic.frag` projects every
pixel into that texture and compares depths; 3x3 samples are averaged (PCF) for a soft edge.
Shadow acne is removed with front-face culling in the depth pass and a small slope-scaled bias.
The room is a cut-away, so a ceiling and a right wall are drawn **only into the shadow
map**: the camera can look in, but the sunlight enters only through the window.

**Lamp shadows** (`PointShadowMap.cpp`, `point_shadow.*`, `basic.frag`). The desk lamp is a
point light, so it shines in every direction and one flat shadow map is not enough. While
the lamp is on, the scene is drawn six times from the bulb into the faces of a depth cube map
(90 degree field of view each); every texel stores the distance from the lamp to the closest
surface. `basic.frag` looks the cube map up with the direction from the lamp to the pixel and
compares the two distances, averaging 8 samples for a soft edge. The lamp's own parts are left
out of this pass, because the bulb sits inside the shade. The desk top now blocks the light,
so the floor under the desk stays dark at night.

**Textures** (`Texture.cpp`, `basic.vert`, `basic.frag`). Images are loaded with stb_image,
uploaded once at start-up and given mipmaps (`glGenerateMipmap`) with trilinear filtering
(`GL_LINEAR_MIPMAP_LINEAR`) and `GL_REPEAT`. Every cube face has UVs from 0 to 1; the vertex
shader multiplies them by the size of that face divided by the tile size, so one 1 m tile
repeats 6 times on the 6 m floor and the texture is never stretched. The colour texture
uses unit 0 (the shadow map is on unit 1) and is multiplied by the object colour, so the
same wallpaper or wood grain can be tinted per object. The textures themselves are made
by `tools/make_textures.py`.

**Scene graph** (`Room.h`, `Room.cpp`, `RoomCat.cpp`). A `SceneNode` keeps a local transform
and a list of children; the world matrix of a part is its parent's world matrix times its own
local matrix. The lamp is a chain base → stem → pivot → arm → shade: `[` and `]` rotate the pivot,
and the arm, the shade and the light position all follow. The light position is read from the
shade's world matrix, so the light always stays inside the shade.

**Fairy lights** (`FairyLights.cpp`, `basic.frag`). The string hangs from hooks 10 cm in front
of the back and left walls; between two hooks the wire sags like a parabola,
y = y0 − 4·sag·t(1 − t). Each of the 12 bulbs is a point light: `basic.frag` adds a diffuse term
with a fast fall-off, 1 / (1 + d + 2.5 d²), so a bulb only lights the wall and the bed close to it.
Bulbs are not shadowed (12 more cube maps would cost too much for such small lights). When the
string switches on, bulb *i* starts after 0.12·*i* s, so a wave runs along it, and every bulb
twinkles slightly. When all bulbs are dark the shader loop is skipped.

**Glowing surfaces** (`uEmissive` in `basic.frag`). An emissive colour is added after all
lighting, so a surface can glow on its own: the bulbs, the lamp shade while the lamp is on, and
the laptop screen. Everything else sends black.

**HUD** (`Hud.cpp`, `TimeControls.cpp`). A second, flat shader draws coloured
rectangles and text on top of the scene with alpha blending. Letters come from
stb_easy_font, which builds every character from small quads, so no font texture is needed.
The time slider is painted with the sky colour of every hour.

## Project structure

```
src/
  main.cpp           window, input, render loop
  Camera.*           orbit camera
  Shader.*           loads and compiles GLSL programs
  Mesh.*             VAO/VBO/EBO, cube mesh with UVs
  Texture.*          stb_image loading, mipmaps
  Room.*             room, furniture, scene graph (lamp)
  RoomCat.cpp        the cat (part of Room)
  DayNight.*         time of day -> sun/moon direction and colours
  ShadowMap.*        depth framebuffer for the sun
  PointShadowMap.*   depth cube map for the desk lamp
  FairyLights.*      fairy lights: wire, bulbs, 12 point lights
  WindowSky.*        procedural sky behind the window
  Hud.*              2D overlay: rectangles and text
  TimeControls.*     time keys, time slider, help panel
  Benchmark.*        FPS benchmark (key B)
shaders/
  basic.vert/.frag   Blinn-Phong, textures, sun + lamp shadows, fairy lights, glow
  shadow_depth.*     depth pass for the sun
  point_shadow.*     depth pass for the lamp (cube map)
  window_sky.*, window_plane.vert   sky through the window
  hud.vert/.frag     2D overlay
textures/            our own textures, made by tools/make_textures.py
external/
  glad/              OpenGL loader
  stb/               stb_easy_font, stb_image
```

## Performance

Measured with the built-in benchmark (key **B**): 20 seconds, vsync off, the camera
makes one full circle while the clock runs through a whole day. The program prints the
table row to the console.

| GPU | Resolution | Avg FPS | Min FPS | Avg frame time |
|-----|------------|---------|---------|----------------|
| llvmpipe (software rendering, no GPU) | 1280x720 | 35 | 20 | 28.52 ms |
| Apple M1 (MacBook Air, Baxa) | 2880x1800 | 337 | 142 | 2.96 ms |

The software-rendering row is the worst case (the CPU does all the GPU work).
Rows for the team's laptops are added after running the benchmark on them.

## Screenshots

| | |
|---|---|
| ![Textured room](docs/screenshots/15-textured-room.png) | ![Sunset](docs/screenshots/12-sunset-warm-light.png) |
| ![Fairy lights and the glowing lamp at night](docs/screenshots/18-fairy-lights-and-glowing-lamp.png) | ![Lamp shadows at night](docs/screenshots/17-night-lamp-shadows.png) |

Earlier progress screenshots are in `docs/screenshots/`.

## AI Use

We used Claude (Anthropic) as an assistant in this project.

- **Planning (whole team):** dividing the work between members, structuring the OpenGL project,
  breaking the Final Project requirements into GitHub issues, reviewing the README.
- **Code (Baxa):** Claude wrote the first version of the day/night cycle, time controls and HUD,
  sun shadow mapping, the FPS benchmark, the fairy lights and the glowing lamp shade, and moved the
  window sky and the cat into their own files. Baxa built and ran every change on macOS, checked it
  against the requirements and asked for changes (room brightness after the shadows, size of the
  help panel, brightness of the fairy lights); he can explain every part of this code.

Every member adds a line about their own use of AI; the same note is in the report.

## Credits

Libraries and references are listed in [CREDITS.md](CREDITS.md).
