#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;

out vec4 FragColor;

uniform vec3 uColor;
uniform vec3 uViewPos;
uniform vec3 uLightDir;     // direction towards the sun
uniform vec3 uLightColor;
uniform vec3 uLampPos;
uniform vec3 uLampColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uViewPos - vWorldPos);

    vec3 L = normalize(uLightDir);
    vec3 H = normalize(L + V);

    vec3 ambient = 0.25 * uColor;

    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * uColor * uLightColor;

    float spec = diff > 0.0 ? pow(max(dot(N, H), 0.0), 32.0) : 0.0;
    vec3 specular = 0.3 * spec * uLightColor;

    vec3 lampVector = uLampPos - vWorldPos;
    float distanceToLamp = length(lampVector);
    vec3 lampDirection = normalize(lampVector);
    vec3 lampHalfway = normalize(lampDirection + V);

    float lampDiff = max(dot(N, lampDirection), 0.0);
    float attenuation = 1.0 / (1.0 + 0.22 * distanceToLamp
                                  + 0.20 * distanceToLamp * distanceToLamp);

    vec3 lampDiffuse = lampDiff * uColor * uLampColor * attenuation;

    float lampSpec = lampDiff > 0.0
        ? pow(max(dot(N, lampHalfway), 0.0), 32.0)
        : 0.0;
    vec3 lampSpecular = 0.2 * lampSpec * uLampColor * attenuation;

    FragColor = vec4(ambient + diffuse + specular
                     + lampDiffuse + lampSpecular, 1.0);
}