#include <glm/gtc/matrix_transform.hpp>

#include "LightSource.h"
#include "Camera.h"
#include "ShaderProgram.h"

LightSource::LightSource(glm::vec3 color, glm::vec3 ambient) :
        color(color), ambient(ambient)
{}

DirectionalLight::DirectionalLight() :
    LightSource(),
    direction(glm::vec3(0.0f, -1.0f, 0.0f))
{}

DirectionalLight::DirectionalLight(glm::vec3 dir, glm::vec3 color, glm::vec3 ambient) :
        LightSource(color, ambient), direction(dir)
{}

void DirectionalLight::setToShader(const ShaderProgram &shader) {
    shader.SetUniform("directionalLight.direction", direction);
    shader.SetUniform("directionalLight.color", color);
    shader.SetUniform("directionalLight.ambient", ambient);
}

Lights::Lights(int w, int h, glm::vec3 light_pos) :
    width(w), height(h), lightPos(light_pos)
{
    lightProjection = glm::perspective(glm::radians(45.0f), (float) width / (float) height, 0.1f, 100.0f);
    lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;
    directionalLights.push_back(DirectionalLight());
}

void Lights::setLightSourceToShader(const ShaderProgram &shader, bool to_depth) {
    if (!to_depth) {
        shader.SetUniform("lightPos", lightPos);
        for (auto &dl : directionalLights)
            dl.setToShader(shader);
    }
    shader.SetUniform("lightSpaceMatrix", lightSpaceMatrix);
}