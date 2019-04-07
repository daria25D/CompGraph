#version 330 core
out vec4 FragColor;

float zNear = 0.1f;
float zFar  = 100.0f;

float LinearizeDepth(float depth)
{
    // преобразуем обратно в NDC
    float z = depth * 2.0 - 1.0;
    return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}

void main()
{
    // деление на zFar для лучшей наглядности
    float depth = LinearizeDepth(gl_FragCoord.z) / zFar;
    FragColor = vec4(vec3(depth), 1.0);
}