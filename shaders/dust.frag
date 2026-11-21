#version 330 core

in float vAlpha;

out vec4 FragColor;

void main() {
    vec2 point = gl_PointCoord * 2.0 - 1.0;
    float distanceFromCenter = dot(point, point);

    float alpha = exp(-distanceFromCenter * 3.8) * vAlpha;

    if (alpha < 0.008) {
        discard;
    }

    FragColor = vec4(1.0, 0.91, 0.76, alpha);
}