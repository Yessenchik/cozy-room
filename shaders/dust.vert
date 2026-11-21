#version 330 core

layout (location = 0) in vec3 aOffset;
layout (location = 1) in float aSize;
layout (location = 2) in float aPhase;

uniform mat4 uView;
uniform mat4 uProjection;
uniform vec3 uWindowOrigin;
uniform float uTime;
uniform float uViewportHeight;
uniform float uSunlightIntensity;

out float vAlpha;

void main() {
    float driftX = sin(uTime * 0.55 + aPhase) * 0.025;
    float driftZ = cos(uTime * 0.42 + aPhase) * 0.020;
    float driftY = sin(uTime * 0.35 + aPhase) * 0.018;

    vec3 worldPosition = uWindowOrigin + aOffset;
    worldPosition += vec3(driftX, driftY, driftZ);

    vec4 viewPosition = uView * vec4(worldPosition, 1.0);
    gl_Position = uProjection * viewPosition;

    float distanceToCamera = max(abs(viewPosition.z), 0.1);
    gl_PointSize = clamp(
        aSize * uViewportHeight / distanceToCamera,
        1.2,
        4.5
    );

    float variation = 0.5 + 0.5 * sin(uTime * 0.65 + aPhase);
    vAlpha = (0.08 + 0.10 * variation) * uSunlightIntensity;
}