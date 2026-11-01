#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;

uniform mat4 uModel;        // object -> world (move, rotate, stretch the object)
uniform mat4 uView;         // world -> camera
uniform mat4 uProjection;   // camera -> screen (perspective)

void main() {
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
}
