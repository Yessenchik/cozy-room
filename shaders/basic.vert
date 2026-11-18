#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat3 uNormalMatrix;
uniform mat4 uLightSpace;   // for shadow mapping
uniform vec3 uTexRepeat;    // object size divided by the texture tile size, per axis

out vec3 vWorldPos;
out vec3 vNormal;
out vec4 vLightSpacePos;
out vec2 vTexCoord;

// The cube UVs go 0..1 on every face. Each face spans two of the object's axes,
// so the UVs are scaled by those two sizes: a 6 m floor repeats the tile 6 times
// along x and z, while the 0.1 m thick edge of a wall repeats it only 0.1 times.
vec2 faceRepeat(vec3 n) {
    vec3 a = abs(n);
    if (a.y > 0.5) return uTexRepeat.xz;   // top / bottom
    if (a.x > 0.5) return uTexRepeat.zy;   // left / right
    return uTexRepeat.xy;                  // front / back
}

void main() {
    vec4 worldPos = uModel * vec4(aPosition, 1.0);
    vWorldPos = worldPos.xyz;
    vNormal = uNormalMatrix * aNormal;
    vLightSpacePos = uLightSpace * worldPos;
    vTexCoord = aTexCoord * faceRepeat(aNormal);
    gl_Position = uProjection * uView * worldPos;
}
