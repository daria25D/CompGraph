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


float ShadowCalculation(vec4 shadowCoord)
{
    // perform perspective divide
    vec3 projCoords = shadowCoord.xyz / shadowCoord.w;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).z;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 0.5 : 0.0;

    return shadow;
}


void main()
{
    vec3 normal = texture(texture_normal1, fs_in.TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space

    vec3 color = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    // ambient
    vec3 ambient = 0.075 * color;
    // diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    vec3 diffuse = max(dot(lightDir, normal), 0.0) * color;
    // specular
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float cosTheta = clamp(dot(normal, lightDir), 0.0, 1.0);
    float cosAlpha = clamp(dot(viewDir, reflectDir), 0.0, 1.0);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);


    vec3 specular = vec3(0.3f) * spec * vec3(texture(texture_specular1, fs_in.TexCoords));


    float shadow = ShadowCalculation(ShadowCoord);
    FragColor = vec4(ambient + (1.0 - shadow) * cosTheta * diffuse + /*(1.0 - shadow)*/ specular * pow(cosAlpha, 5), 1.0);

    //vec3 lightColor = vec3(1.0f);
    //vec3 diffuseColor = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    //float visibility = texture(shadowMap, vec3(ShadowCoord.xy, (ShadowCoord.z)/ShadowCoord.w)).z;
    //FragColor = vec4(visibility * diffuseColor * lightColor, 1.0);
//    vec3 lightPos = vec3(.0f, 2.0f, 6.0f);
//    vec3 color = texture(texture_diffuse1, fs_in.TexCoords).rgb;
//    vec3 normal = normalize(fs_in.Normal);
//    vec3 lightColor = vec3(0.3);
//    // ambient
//    vec3 ambient = 0.3 * color;
//    // diffuse
//    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
//    float diff = max(dot(lightDir, normal), 0.0);
//    vec3 diffuse = diff * lightColor;
//    // specular
//    vec3 viewDir = normalize(-fs_in.FragPos);
//    vec3 reflectDir = reflect(-lightDir, normal);
//    float spec = 0.0;
//    vec3 halfwayDir = normalize(lightDir);
//    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
//    vec3 specular = spec * lightColor;
//    // calculate shadow
//    float shadow = ShadowCalculation(ShadowCoord);
//    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
//
//    FragColor = vec4(lighting, 1.0);
}