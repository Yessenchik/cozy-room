#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;
in vec4 vLightSpacePos;
in vec2 vTexCoord;

out vec4 FragColor;

uniform vec3 uColor;
uniform vec3 uViewPos;
uniform vec3 uLightDir;     // direction towards the sun (or the moon at night)
uniform vec3 uLightColor;   // already includes the intensity for this time of day
uniform vec3 uAmbient;      // sky fill light, changes with the time of day
uniform vec3 uLampPos;
uniform vec3 uLampColor;
uniform sampler2DShadow uShadowMap;   // depth seen from the sun, texture unit 1
uniform sampler2D uTexture;           // colour texture, texture unit 0
uniform bool uUseTexture;             // false for plain-coloured furniture
uniform samplerCube uLampShadowMap;   // distance to the closest surface around the lamp, unit 2
uniform float uLampFarPlane;

// fairy lights: small warm point lights over the bed (no shadows, short range)
#define MAX_FAIRY 12
uniform int uFairyCount;
uniform vec3 uFairyPos[MAX_FAIRY];
uniform vec3 uFairyColor[MAX_FAIRY];   // colour * brightness, black when off

uniform vec3 uEmissive;   // light the surface gives off itself (bulbs); 0 for everything else

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

// 1.0 = the lamp sees this pixel, 0.0 = something is between them.
// The cube map is looked up with the direction from the lamp to the pixel;
// 8 samples around that direction are averaged (PCF), so the edge is soft.
float lampVisibility(vec3 lampToFrag) {
    float current = length(lampToFrag);
    const float bias = 0.03;
    const float radius = 0.015;
    const vec3 offsets[8] = vec3[](
        vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
        vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1));

    float lit = 0.0;
    for (int i = 0; i < 8; ++i) {
        float closest = texture(uLampShadowMap, lampToFrag + offsets[i] * radius * current).r
                        * uLampFarPlane;
        lit += current - bias > closest ? 0.0 : 1.0;
    }
    return lit / 8.0;
}

void main() {
    // the texture is tinted by the object colour, so one texture can serve several objects
    vec3 albedo = uUseTexture ? texture(uTexture, vTexCoord).rgb * uColor : uColor;

    vec3 N = normalize(vNormal);
    vec3 V = normalize(uViewPos - vWorldPos);

    vec3 L = normalize(uLightDir);
    vec3 H = normalize(L + V);

    vec3 ambient = uAmbient * albedo;

    float diff = max(dot(N, L), 0.0);
    float shadow = sunVisibility(N, L);
    vec3 diffuse = shadow * diff * albedo * uLightColor;

    float spec = diff > 0.0 ? pow(max(dot(N, H), 0.0), 32.0) : 0.0;
    vec3 specular = shadow * 0.3 * spec * uLightColor;

    vec3 lampVector = uLampPos - vWorldPos;
    float distanceToLamp = length(lampVector);
    vec3 lampDirection = normalize(lampVector);
    vec3 lampHalfway = normalize(lampDirection + V);

    float lampDiff = max(dot(N, lampDirection), 0.0);
    float attenuation = 1.0 / (1.0 + 0.22 * distanceToLamp
                                  + 0.20 * distanceToLamp * distanceToLamp);

    // the shadow test is skipped when the lamp is off (its colour is black)
    float lampShadow = dot(uLampColor, uLampColor) > 0.0 ? lampVisibility(-lampVector) : 1.0;
    attenuation *= lampShadow;

    vec3 lampDiffuse = lampDiff * albedo * uLampColor * attenuation;

    float lampSpec = lampDiff > 0.0
        ? pow(max(dot(N, lampHalfway), 0.0), 32.0)
        : 0.0;
    vec3 lampSpecular = 0.2 * lampSpec * uLampColor * attenuation;

    // every bulb adds a diffuse term with a fast fall-off, so it only lights
    // the walls and the bed close to it
    vec3 fairy = vec3(0.0);
    for (int i = 0; i < uFairyCount; ++i) {
        vec3 toBulb = uFairyPos[i] - vWorldPos;
        float d = length(toBulb);
        float fairyDiff = max(dot(N, toBulb / d), 0.0);
        float fairyAtt = 1.0 / (1.0 + 1.0 * d + 2.5 * d * d);
        fairy += fairyDiff * fairyAtt * albedo * uFairyColor[i];
    }

    FragColor = vec4(ambient + diffuse + specular
                     + lampDiffuse + lampSpecular
                     + fairy + uEmissive, 1.0);
}