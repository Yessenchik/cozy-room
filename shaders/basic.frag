#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;

out vec4 FragColor;

uniform vec3 uColor;
uniform vec3 uViewPos;
uniform vec3 uLightDir;     // direction towards the sun
uniform vec3 uLightColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);
    vec3 V = normalize(uViewPos - vWorldPos);
    vec3 H = normalize(L + V);

    vec3 ambient = 0.25 * uColor;

    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * uColor * uLightColor;

    float spec = diff > 0.0 ? pow(max(dot(N, H), 0.0), 32.0) : 0.0;
    vec3 specular = 0.3 * spec * uLightColor;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
