#version 330 core

#define NUM_DIR_LIGHTS 1
#define NUM_POINT_LIGHTS 2

out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
//    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    mat3 TBN;
} fs_in;

in Directional_Light {
    vec3 direction;
    vec3 color;
    vec3 ambient;
} directional_light_in[NUM_DIR_LIGHTS];

in Point_Light {
    vec3 position;
    vec3 color;
    vec3 ambient;
    vec3 params;
} point_light_in[NUM_POINT_LIGHTS];

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

vec3 calculateDirectionalLight(int i, vec3 normal, vec3 view_dir, vec3 color) {
    vec3 lightDir = -directional_light_in[i].direction;

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(view_dir, reflectDir), 0.0), 32.0);
//    vec3 halfwayDir = normalize(lightDir + view_dir);
//    float spec = pow(max(dot(view_dir, halfwayDir), 0.0), 32.0);

    vec3 ambient  = directional_light_in[i].ambient;
    vec3 diffuse  = directional_light_in[i].color * diff;
    vec3 specular = vec3(spec); // TODO add specular texture

    return (ambient + diffuse) * color + specular;
}

vec3 calculatePointLight(int i, vec3 normal, vec3 view_dir, vec3 tfrag_pos, vec3 color) {
    vec3 lightDir = normalize(point_light_in[i].position - tfrag_pos);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(view_dir, reflectDir), 0.0), 32.0);
    //TODO can be optimized with normalization before
    float distance = distance(point_light_in[i].position, tfrag_pos);
    float attenuation = 1.0 / (point_light_in[i].params.x + point_light_in[i].params.y * distance +
                               point_light_in[i].params.z * distance * distance);

    vec3 ambient  = point_light_in[i].ambient;
    vec3 diffuse  = point_light_in[i].color * diff;
    vec3 specular = vec3(spec); // TODO add specular texture

    return attenuation * ((ambient + diffuse) * color + specular);
}

float shadowCalculation(vec4 shadowCoord, float bias)
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
    vec3 color = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    // normal
    vec3 normal = texture(texture_normal1, fs_in.TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 lighting = vec3(0.0);
    for (int i = 0; i < NUM_DIR_LIGHTS; i++)
        lighting += calculateDirectionalLight(i, normal, viewDir, color);
    for (int i = 0; i < NUM_POINT_LIGHTS; i++)
        lighting += calculatePointLight(i, normal, viewDir, fs_in.TangentFragPos, color);
    // calculate shadow
    float shadow = shadowCalculation(ShadowCoord, 0.005);
//    vec3 lighting = (ambient + shadow * (diffuse + specular * vec3(texture(texture_specular1, fs_in.TexCoords)))) * color;
    FragColor = vec4(lighting, 1.0);
}