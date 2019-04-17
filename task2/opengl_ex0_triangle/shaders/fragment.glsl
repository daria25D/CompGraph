#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

in vec4 ShadowCoord;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_specular1;
uniform sampler2D shadowMap;

vec2 poissonDisk[4] = vec2[](
  vec2( -0.94201624, -0.39906216 ),
  vec2( 0.94558609, -0.76890725 ),
  vec2( -0.094184101, -0.92938870 ),
  vec2( 0.34495938, 0.29387760 )
);

float ShadowCalculation(vec4 shadowCoord, float bias)
{
    // perform perspective divide
    vec3 projCoords = shadowCoord.xyz / shadowCoord.w;
    projCoords = projCoords * 0.5 + 0.5;
    float shadow = 1.0f;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    for (int i = 0; i < 4; i++) {
        float closestDepth = texture(shadowMap, projCoords.xy + poissonDisk[i]/700.0f).r;
        // get depth of current fragment from light's perspective
        float currentDepth = projCoords.z;
        // check whether current frag pos is in shadow
        if (currentDepth - 0.0001 > closestDepth) {
            shadow -= 0.2f;
        }
    }
    //float shadow = currentDepth - 0.0001 > closestDepth ? 0.6 : 0.0;

    return shadow;
}


void main()
{
    vec3 normal = texture(texture_normal1, fs_in.TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 color = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    vec3 lightColor = vec3(0.7);
    // ambient
    vec3 ambient = 0.125 * color;
    // diffuse
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = spec * lightColor;
    // calculate shadow
    float shadow = ShadowCalculation(ShadowCoord, 0.005);
    vec3 lighting = (ambient + shadow * (diffuse + specular * vec3(texture(texture_specular1, fs_in.TexCoords)))) * color;

    FragColor = vec4(lighting, 1.0);
}