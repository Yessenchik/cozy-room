#version 330 core

layout (location = 0) in vec3 aPosition;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform vec3 uCameraPos;

out vec3 vDirection;

void main() {
    vec4 worldPosition = uModel * vec4(aPosition, 1.0);
    vDirection = worldPosition.xyz - uCameraPos;
    gl_Position = uProjection * uView * worldPosition;
}