#version 330 core
out vec4 FragColor;
//uniform sampler2D shadowMap;
float zNear = 0.1f;
float zFar  = 100.0f;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}

void main()
{
    FragColor = vec4(vec3(LinearizeDepth(gl_FragCoord.z) / zFar), 1.0); // perspective
}