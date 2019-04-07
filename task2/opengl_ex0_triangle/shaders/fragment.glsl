//#version 330 core
//
//in vec2 UV;
//out vec3 color;
//uniform sampler2D myTextureSampler;
//
//void main()
//{
//  //color = vec3(1.0f, 0.0f, 0.0f);
//  color = texture(myTextureSampler, UV).rgb;
//}


#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_specular1;

void main()
{
    vec3 normal = texture(texture_normal1, fs_in.TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space

    vec3 color = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    // ambient
    vec3 ambient = 0.1 * color;
    // diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    vec3 diffuse =  max(dot(lightDir, normal), 0.0) * color;
    // specular
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2f) * spec * vec3(texture(texture_specular1, fs_in.TexCoords));
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}