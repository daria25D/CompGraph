#ifndef MAIN_LIGHTSOURCE_H
#define MAIN_LIGHTSOURCE_H

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <vector>

class ShaderProgram;

class DirectionalLight {
    glm::vec3 direction;
    glm::vec3 color;
    glm::vec3 ambient;
public:
    DirectionalLight();
    explicit DirectionalLight(glm::vec3 dir,
                              glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f),
                              glm::vec3 ambient = glm::vec3(0.2f, 0.2f, 0.2f));
    void setToShader(const ShaderProgram &shader);
};
//TODO add class from which other light sources can inherit
class LightSource {
    glm::vec3 lightPos;
    int width, height;

    glm::mat4 lightProjection, lightView, lightSpaceMatrix;

public:
    explicit LightSource(int w, int h, glm::vec3 light_pos);
    void setLightSourceToShader(const ShaderProgram &shader, bool to_depth = false);
};

struct Lights {
    std::vector<DirectionalLight> directionalLights;
//    vector<LightSource> lightSources;
    // TODO add methods
};

#endif //MAIN_LIGHTSOURCE_H
