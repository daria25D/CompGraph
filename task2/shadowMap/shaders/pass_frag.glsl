#version 330 core
out vec4 FragColor;
uniform sampler2D shadowMap;
float zNear = 0.1f;
float zFar  = 100.0f;
in vec2 UV;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}

void main()
{
    float depthValue = texture(shadowMap, UV).r;
    FragColor = vec4(vec3(LinearizeDepth(depthValue) / zFar), 1.0); // perspective
}