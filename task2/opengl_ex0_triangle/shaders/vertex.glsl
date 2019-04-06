#version 330 core

// Input vertex data, different for all executions of this shader
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;

uniform mat4 model;
uniform mat4 proj;
uniform mat4 view;

out vec2 UV;

void main() {
    gl_Position = proj * view * model * vec4(vertexPosition_modelspace, 1);  // Output position of the vertex, in clip space
    UV = vertexUV;
}