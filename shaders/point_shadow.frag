#version 330 core
in vec3 vWorldPos;

uniform vec3 uLightPos;
uniform float uFarPlane;

// Store the real distance to the light (0..1), not the perspective depth,
// so basic.frag can compare it directly with its own distance to the lamp.
void main() {
    gl_FragDepth = length(vWorldPos - uLightPos) / uFarPlane;
}
