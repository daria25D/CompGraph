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
    virtual void setToShader(const ShaderProgram &shader, int i){}
};

class DirectionalLight : LightSource {
    glm::vec3 direction;
public:
    DirectionalLight();
    explicit DirectionalLight(glm::vec3 dir,
                              glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f),
                              glm::vec3 ambient = glm::vec3(0.2f, 0.2f, 0.2f));
    void setToShader(const ShaderProgram &shader, int i) override;
};

class PointLight : LightSource {
    glm::vec3 position;
    //dist=100 	c=1.0 	l=0.045 	q=0.0075
    float constant;
    float linear;
    float quadratic;
public:
    PointLight();
    explicit PointLight(glm::vec3 pos, float constant = 1.0f, float linear = 0.045f, float quadratic = 0.0075f,
                        glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f),
                        glm::vec3 ambient = glm::vec3(0.2f, 0.2f, 0.2f));
    void setToShader(const ShaderProgram &shader, int i) override;
};

class Lights {
    glm::vec3 lightPos;
    int width, height;

    glm::mat4 lightProjection, lightView, lightSpaceMatrix;
    std::vector<DirectionalLight> directionalLights;
    std::vector<PointLight> pointLights;

public:
    explicit Lights(int w, int h, glm::vec3 light_pos);
    void setLightSourceToShader(const ShaderProgram &shader, bool to_depth = false);
};

#endif //MAIN_LIGHTSOURCE_H
