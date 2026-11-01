#version 330 core
out vec4 FragColor;

uniform vec3 uColor;   // flat colour for now; lighting comes in a later step

void main() {
    FragColor = vec4(uColor, 1.0);
}
