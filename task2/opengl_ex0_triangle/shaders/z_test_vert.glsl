#version 330

layout(location = 0) in vec3 vertexPosition;

uniform mat4 model;
uniform mat4 proj;
uniform mat4 view;

void main() {
    gl_Position = proj * view * model * vec4(vertexPosition, 1.0);
}
