#version 330 core

in vec3 vDirection;

uniform float uHour;
uniform float uTime;
uniform vec3 uSunDirection;
uniform vec3 uMoonDirection;

out vec4 FragColor;

float hash21(vec2 p) {
    p = fract(p * vec2(123.34, 345.45));
    p += dot(p, p + 34.345);
    return fract(p.x * p.y);
}

void main() {
    vec3 direction = normalize(vDirection);
    float hour = mod(uHour, 24.0);

    float daylight =
        smoothstep(5.5, 7.5, hour) *
        (1.0 - smoothstep(18.0, 20.5, hour));

    float dawn = exp(-pow((hour - 6.6) / 1.4, 2.0));
    float dusk = exp(-pow((hour - 18.4) / 1.4, 2.0));
    float twilight = clamp(
        max(dawn, dusk) * (1.0 - daylight),
        0.0,
        1.0
    );
    float night = clamp(
        1.0 - daylight - twilight * 0.85,
        0.0,
        1.0
    );

    float height = clamp(direction.y * 0.5 + 0.5, 0.0, 1.0);

    vec3 nightSky = mix(
        vec3(0.13, 0.08, 0.20),
        vec3(0.015, 0.02, 0.08),
        height
    );

    vec3 daySky = mix(
        vec3(0.82, 0.90, 1.0),
        vec3(0.12, 0.43, 0.82),
        height
    );

    vec3 twilightSky = mix(
        vec3(1.0, 0.42, 0.19),
        vec3(0.29, 0.18, 0.40),
        height
    );

    vec3 color = mix(nightSky, daySky, daylight);
    color = mix(color, twilightSky, twilight);

    float sunDot = dot(direction, normalize(uSunDirection));
    float sunDisk = smoothstep(
        cos(0.055),
        cos(0.035),
        sunDot
    );
    float sunGlow = exp(-max(0.0, 1.0 - sunDot) * 90.0) * daylight;

    color += vec3(1.0, 0.55, 0.22) * sunGlow * 0.45;
    color = mix(color, vec3(1.0, 0.82, 0.52), sunDisk * daylight);

    float moonDot = dot(direction, normalize(uMoonDirection));
    float moonDisk = smoothstep(
        cos(0.045),
        cos(0.030),
        moonDot
    );

    vec3 moonCutoutDirection = normalize(
        uMoonDirection + vec3(0.018, 0.006, 0.0)
    );
    float moonCutoutDot = dot(direction, moonCutoutDirection);
    float moonCutout = smoothstep(
        cos(0.040),
        cos(0.026),
        moonCutoutDot
    );

    float moon = moonDisk * (1.0 - moonCutout) * night;
    color = mix(color, vec3(0.88, 0.92, 1.0), moon);

    float longitude =
        atan(direction.z, direction.x) / 6.2831853 + 0.5;
    float latitude =
        asin(clamp(direction.y, -1.0, 1.0)) / 3.14159265 + 0.5;

    vec2 grid = vec2(longitude, latitude) * vec2(120.0, 60.0);
    vec2 cell = floor(grid);
    vec2 local = fract(grid);

    float randomValue = hash21(cell);
    vec2 starPosition = vec2(
        hash21(cell + 17.7),
        hash21(cell + 91.3)
    );

    float starDistance = length(
        (local - starPosition) * vec2(120.0, 60.0)
    );

    float starShape = 1.0 - smoothstep(0.035, 0.12, starDistance);
    float starExists = step(0.992, randomValue);
    float twinkle =
        0.70 +
        0.30 * sin(
            uTime * (1.5 + randomValue * 4.0) +
            randomValue * 6.28
        );

    float starsAboveHorizon = smoothstep(-0.02, 0.18, direction.y);

    color +=
        vec3(0.75, 0.84, 1.0) *
        starShape *
        starExists *
        night *
        starsAboveHorizon *
        twinkle;

    FragColor = vec4(color, 1.0);
}