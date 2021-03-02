#ifndef MAIN_LIGHTSOURCE_H
#define MAIN_LIGHTSOURCE_H

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <vector>

class ShaderProgram;

class LightSource {
protected:
    glm::vec3 color;
    glm::vec3 ambient;
public:
    LightSource(glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f),
                         glm::vec3 ambient = glm::vec3(0.2f, 0.2f, 0.2f));
    virtual void setToShader(const ShaderProgram &shader){}
};

class DirectionalLight : LightSource {
    glm::vec3 direction;
public:
    DirectionalLight();
    explicit DirectionalLight(glm::vec3 dir,
                              glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f),
                              glm::vec3 ambient = glm::vec3(0.2f, 0.2f, 0.2f));
    void setToShader(const ShaderProgram &shader) override;
};

class Lights {
    glm::vec3 lightPos;
    int width, height;

    glm::mat4 lightProjection, lightView, lightSpaceMatrix;
    std::vector<DirectionalLight> directionalLights;

public:
    explicit Lights(int w, int h, glm::vec3 light_pos);
    void setLightSourceToShader(const ShaderProgram &shader, bool to_depth = false);
};

#endif //MAIN_LIGHTSOURCE_H
