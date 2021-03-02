#include <glm/gtc/matrix_transform.hpp>

#include "LightSource.h"
#include "Camera.h"
#include "ShaderProgram.h"

DirectionalLight::DirectionalLight() :
    direction(glm::vec3(0.0f, -1.0f, 0.0f)),
    color(glm::vec3(1.0f, 1.0f, 1.0f)),
    ambient(glm::vec3(0.2f, 0.2f, 0.2f))
{}

DirectionalLight::DirectionalLight(glm::vec3 dir, glm::vec3 color, glm::vec3 ambient) :
        direction(dir), color(color), ambient(ambient)
{}

void DirectionalLight::setToShader(const ShaderProgram &shader) {
    shader.SetUniform("directionalLight.direction", direction);
    shader.SetUniform("directionalLight.color", color);
    shader.SetUniform("directionalLight.ambient", ambient);
}

LightSource::LightSource(int w, int h, glm::vec3 light_pos) :
    width(w), height(h), lightPos(light_pos)
{
    lightProjection = glm::perspective(glm::radians(45.0f), (float) width / (float) height, 0.1f, 100.0f);
    lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;
}

void LightSource::setLightSourceToShader(const ShaderProgram &shader, bool to_depth) {
    if (!to_depth)
        shader.SetUniform("lightPos", lightPos);
    shader.SetUniform("lightSpaceMatrix", lightSpaceMatrix);
}