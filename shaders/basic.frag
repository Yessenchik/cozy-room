#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;
in vec4 vLightSpacePos;

out vec4 FragColor;

uniform vec3 uColor;
uniform vec3 uViewPos;
uniform vec3 uLightDir;     // direction towards the sun (or the moon at night)
uniform vec3 uLightColor;   // already includes the intensity for this time of day
uniform vec3 uAmbient;      // sky fill light, changes with the time of day
uniform vec3 uLampPos;
uniform vec3 uLampColor;
uniform sampler2DShadow uShadowMap;   // depth seen from the sun, texture unit 1

// 1.0 = fully lit by the sun, 0.0 = fully in shadow.
// PCF: 3x3 samples around the pixel are averaged, so the shadow edge is soft.
float sunVisibility(vec3 N, vec3 L) {
    vec3 p = vLightSpacePos.xyz / vLightSpacePos.w * 0.5 + 0.5;
    if (p.z > 1.0)
        return 1.0;

    // a small bias, a bit bigger on surfaces that the light hits at a steep angle
    float bias = max(0.0015 * (1.0 - dot(N, L)), 0.0004);
    vec2 texel = 1.0 / vec2(textureSize(uShadowMap, 0));

    float lit = 0.0;
    for (int x = -1; x <= 1; ++x)
        for (int y = -1; y <= 1; ++y)
            lit += texture(uShadowMap, vec3(p.xy + vec2(x, y) * texel, p.z - bias));
    return lit / 9.0;
}

void main() {
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uViewPos - vWorldPos);

    vec3 L = normalize(uLightDir);
    vec3 H = normalize(L + V);

    vec3 ambient = uAmbient * uColor;

    float diff = max(dot(N, L), 0.0);
    float shadow = sunVisibility(N, L);
    vec3 diffuse = shadow * diff * uColor * uLightColor;

    float spec = diff > 0.0 ? pow(max(dot(N, H), 0.0), 32.0) : 0.0;
    vec3 specular = shadow * 0.3 * spec * uLightColor;

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