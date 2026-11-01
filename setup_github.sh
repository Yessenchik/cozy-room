#!/usr/bin/env bash
# Cozy Room - creates labels, milestones, issues and a GitHub Project board.
# Run ONCE from inside your cloned repo (Windows: use Git Bash), after:
#   gh auth login
#   gh auth refresh -s project
set -e

# ---- fill in your teammates' GitHub usernames -------------------------------
YESSEN="Yessenchik"
BAXA="dzha11"
ARUZHAN="aruwksc"
AIDANA="ai1_moon"
# ------------------------------------------------------------------------------

REPO=$(gh repo view --json nameWithOwner -q .nameWithOwner)
OWNER=${REPO%%/*}
echo "Setting up $REPO"

# ---- labels ------------------------------------------------------------------
label() { gh label create "$1" --color "$2" --description "$3" --force >/dev/null; }
label "phase-1-room"      "5319e7" "Yessen: setup, engine, empty room"
label "phase-2-furniture" "0e8a16" "Aidana + Aruzhan: furniture, textures, lamp"
label "phase-3-daynight"  "fbca04" "Baxa: day/night cycle and final polish"
label "stretch"           "d93f0b" "Stretch goal"
label "docs"              "c5def5" "README, CREDITS, report, slides, video"

# ---- milestones --------------------------------------------------------------
ms() { gh api "repos/$REPO/milestones" -f title="$1" -f description="$2" >/dev/null 2>&1 || true; }
M7="Phase 1 - empty room (Mon-Tue)"
M8="Phase 2 - furniture (Wed-Fri)"
M9="Phase 3 - day/night and polish (weekend)"
MS="Final submission"
ms "$M7" "Phase 1: window, camera, light, walls, floor, window"
ms "$M8" "Phase 2: furniture, textures, desk lamp"
ms "$M9" "Phase 3: day/night cycle, controls, help, final checks"
ms "$MS" "Report, slides, demo video, tag v1.0"

# ---- project board -----------------------------------------------------------
PROJECT=$(gh project create --owner "$OWNER" --title "Cozy Room" --format json -q .number)
gh project link "$PROJECT" --owner "$OWNER" --repo "$REPO" >/dev/null 2>&1 || true
echo "Project #$PROJECT created"

# ---- issues ------------------------------------------------------------------
issue() {  # title, assignee, label, milestone, body
  # If an assignee hasn't accepted the invite yet, create the issue unassigned
  # (assign it later on GitHub) instead of stopping the whole script.
  url=$(gh issue create --title "$1" --assignee "$2" --label "$3" --milestone "$4" --body "$5" 2>/dev/null) ||
  { echo "  (could not assign '$2' - created unassigned)";
    url=$(gh issue create --title "$1" --label "$3" --milestone "$4" --body "$5"); }
  gh project item-add "$PROJECT" --owner "$OWNER" --url "$url" >/dev/null
  echo "  $url"
}

# ===== Phase 1 - Yessen: setup + empty room ===================================
issue "Set up CMake project, GLFW window and GLAD" "$YESSEN" "phase-1-room" "$M7" \
"- [ ] CMakeLists.txt with GLFW, GLM, GLAD
- [ ] 1280x720 window, OpenGL 3.3 core profile
- [ ] .gitignore (no build files committed)
- [ ] Builds the same way as our course assignments (R1, R8)"

issue "Shader class and Mesh class (VAO/VBO/EBO cube)" "$YESSEN" "phase-1-room" "$M7" \
"- [ ] Shader: load .vert/.frag from files, compile, link, print errors
- [ ] Mesh: VAO/VBO/EBO, uploaded once (GL_STATIC_DRAW)
- [ ] Cube with 24 vertices (one normal per face)"

issue "Orbit camera with mouse and keyboard" "$YESSEN" "phase-1-room" "$M7" \
"- [ ] Yaw / pitch / distance around the room centre
- [ ] Right-drag to rotate, scroll to zoom, WASD/QE
- [ ] Movement multiplied by dt (frame-rate independent) (R6, C1)"

issue "Basic Blinn-Phong lighting with the sun" "$YESSEN" "phase-1-room" "$M7" \
"- [ ] Ambient + diffuse + specular per fragment (R4)
- [ ] One directional light (sun), fixed for now - Baxa makes it move later
- [ ] Normal matrix = inverse-transpose of model"

issue "Empty room: floor, two walls, window" "$YESSEN" "phase-1-room" "$M7" \
"- [ ] Floor and left + back walls (diorama open to the viewer)
- [ ] Window opening in the back wall with a frame and sill
- [ ] A simple way for teammates to add objects (one function / list)"

issue "FPS counter and README skeleton" "$YESSEN" "phase-1-room,docs" "$M7" \
"- [ ] FPS in the window title, vsync off (R7)
- [ ] README: build steps, controls, team and phases"

# ===== Phase 2 - Aidana: bed side of the room + textures ======================
issue "Bed: frame, mattress, blanket, pillow, headboard" "$AIDANA" "phase-2-furniture" "$M8" \
"- [ ] Built from cubes, placed against the left wall
- [ ] Consistent cozy colour palette (B1)"

issue "Bookshelf with books, rug, potted plant" "$AIDANA" "phase-2-furniture" "$M8" \
"- [ ] Shelf on the left wall with 6-9 books of different heights
- [ ] Rug on the floor, plant in the corner (sphere + cylinder)"

issue "Textures: wood floor, walls, rug with mipmaps" "$AIDANA" "phase-2-furniture" "$M8" \
"- [ ] Texture class: stb_image, GL_REPEAT, mipmaps (R5)
- [ ] UVs on meshes, texture sampled in the shader
- [ ] Note each texture's author and license for CREDITS"

# ===== Phase 2 - Aruzhan: desk side of the room + lamp ========================
issue "Scene graph: parent-child objects" "$ARUZHAN" "phase-2-furniture" "$M8" \
"- [ ] SceneNode: world = parent.world * local (R3)
- [ ] Moving the desk moves everything on it"

issue "Desk and chair under the window" "$ARUZHAN" "phase-2-furniture" "$M8" \
"- [ ] Desk top + 4 legs, chair seat + back + legs
- [ ] Mug and spinning globe on the desk (children of the desk)"

issue "Desk lamp: hierarchy + second light, toggle with L" "$ARUZHAN" "phase-2-furniture" "$M8" \
"- [ ] Lamp: base -> arm -> head -> bulb, arm rotates around its joint
- [ ] Point light with attenuation at the bulb (R4: 2+ lights, one dynamic)
- [ ] L = on/off, arrow keys = aim the lamp head"

# ===== Phase 3 - Baxa: day/night + finish ====================================
issue "Stretch goal 1: day/night cycle (start early, in parallel)" "$BAXA" "phase-3-daynight,stretch" "$M9" \
"Start on Monday on your own branch - this class does not need the furniture.
- [ ] DayNightCycle class: time of day -> sun direction, light colour, ambient
- [ ] Warm light at sunrise/sunset, dark blue at night
- [ ] Sky colour changes, stars at night
- [ ] Plug it into the room once phase 2 is merged"

issue "Time controls and on-screen help" "$BAXA" "phase-3-daynight" "$M9" \
"- [ ] P = pause, [ ] = slower/faster, 1-4 = morning/noon/sunset/night
- [ ] C = switch orbit / first-person camera
- [ ] Controls help visible in the program (C3)"

issue "Stretch goal 2 (team to choose)" "$BAXA" "phase-3-daynight,stretch" "$M9" \
"2 working stretch goals = 8 points (A5). Pick one by Wednesday:
- [ ] Shadows from the sun (shadow mapping), or
- [ ] Loaded 3D model (OBJ) for one object, or
- [ ] ImGui panel with time slider and lamp toggle"

issue "Final check: FPS table, README, CREDITS, tag v1.0" "$BAXA" "phase-3-daynight,docs" "$M9" \
"- [ ] FPS table: hardware + FPS at 1280x720 (B3, report)
- [ ] README complete: build, controls, features, who did what
- [ ] CREDITS.md: every library, texture, tutorial with license + AI Use note
- [ ] Tag v1.0 on the submission date"

# ===== Shared ================================================================
issue "Written report (all members)" "$YESSEN,$BAXA,$ARUZHAN,$AIDANA" "docs" "$MS" \
"Each member writes the sections about their own phase.
- [ ] Introduction
- [ ] Literature review: 15+ journal sources (4 each), compared not listed
- [ ] Project description
- [ ] Technical implementation with equations
- [ ] Graphics and rendering, screenshots of every feature, FPS table
- [ ] Conclusion and future work
- [ ] References (IEEE or APA) + AI Use note"

issue "Slides: 8-12, every member presents their part" "$YESSEN,$BAXA,$ARUZHAN,$AIDANA" "docs" "$MS" \
"- [ ] Problem, pipeline and architecture, one key shader explained, results, lessons learned
- [ ] ~2 minutes per member"

issue "Demo video: 3-6 min, 1080p, FPS visible" "$YESSEN,$BAXA,$ARUZHAN,$AIDANA" "docs" "$MS" \
"- [ ] Shows R1-R7 and both stretch goals
- [ ] Voice-over or captions naming each feature"

echo "Done. Open the board with: gh project view $PROJECT --owner $OWNER --web"
