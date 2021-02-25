#ifndef MAIN_LIGHTSOURCE_H
#define MAIN_LIGHTSOURCE_H

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

class ShaderProgram;

class LightSource {
    glm::vec3 lightPos;
    int width, height;

    glm::mat4 lightProjection, lightView, lightSpaceMatrix;

public:
    explicit LightSource(int w, int h, glm::vec3 light_pos);
    void setLightSourceToShader(const ShaderProgram &shader);
};

#endif //MAIN_LIGHTSOURCE_H
