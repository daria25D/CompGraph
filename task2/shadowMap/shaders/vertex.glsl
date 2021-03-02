#version 330 core

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    vec3 ambient;
};

// Input vertex data, different for all executions of this shader
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;
layout(location = 3) in vec3 vertexTangent;
layout(location = 4) in vec4 vertexBitangent;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform mat4 lightSpaceMatrix;
uniform mat4 model;
uniform mat4 proj;
uniform mat4 view;
uniform DirectionalLight directionalLight;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
//    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    mat3 TBN;
} vs_out;

out Directional_Light {
    vec3 direction;
    vec3 color;
    vec3 ambient;
} directional_light_out;

out vec4 ShadowCoord;

void main()
{
    vs_out.FragPos = vec3(model * vec4(vertexPosition, 1.0));
    vs_out.TexCoords = vertexUV;
    vs_out.Normal = vertexNormal;
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * vertexTangent);
    vec3 N = normalize(normalMatrix * vertexNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    mat3 TBN = transpose(mat3(T, B, N));
//    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos  = TBN * viewPos;
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;
    vs_out.TBN = TBN;
    directional_light_out.direction = TBN * directionalLight.direction;
    directional_light_out.color = directionalLight.color;
    directional_light_out.ambient = directionalLight.ambient;
//    mat4 bias = mat4(
//    			0.5, 0.0, 0.0, 0.0,
//    			0.0, 0.5, 0.0, 0.0,
//    			0.0, 0.0, 0.5, 0.0,
//    			0.5, 0.5, 0.5, 1.0
//    );

    ShadowCoord = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    gl_Position = proj * view * vec4(vs_out.FragPos, 1.0);
}