#include <glm/gtc/matrix_transform.hpp>
#include <boost/format.hpp>

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

void DirectionalLight::setToShader(const ShaderProgram &shader, int i) {
    shader.SetUniform((boost::format("directionalLight[%d].direction") % i).str(), direction);
    shader.SetUniform((boost::format("directionalLight[%d].color") % i).str(), color);
    shader.SetUniform((boost::format("directionalLight[%d].ambient") % i).str(), ambient);
}

PointLight::PointLight()  :
        LightSource(),
        position(glm::vec3(0.0f, 5.0f, 0.0f)),
        constant(1.0), linear(0.22), quadratic(0.2)
{}

PointLight::PointLight(glm::vec3 pos, float constant, float linear, float quadratic,
                       glm::vec3 color, glm::vec3 ambient) :
    LightSource(color, ambient), position(pos), constant(constant), linear(linear), quadratic(quadratic)
{}

void PointLight::setToShader(const ShaderProgram &shader, int i) {
    shader.SetUniform((boost::format("pointLight[%d].position") % i).str(), position);
    shader.SetUniform((boost::format("pointLight[%d].color") % i).str(), color);
    shader.SetUniform((boost::format("pointLight[%d].ambient") %i).str(), ambient);
    shader.SetUniform((boost::format("pointLight[%d].params") %i).str(), glm::vec3(constant, linear, quadratic));
}

Lights::Lights(int w, int h, glm::vec3 light_pos) :
    width(w), height(h), lightPos(light_pos)
{
    lightProjection = glm::perspective(glm::radians(45.0f), (float) width / (float) height, 0.1f, 100.0f);
    lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;
    directionalLights.push_back(DirectionalLight());
//    directionalLights.push_back(DirectionalLight(glm::vec3(0.0f, 0.0, 1.0)));
    pointLights.push_back(PointLight(glm::vec3(-7.0f, 6.5f, 8.0f)));
    pointLights.push_back(PointLight(glm::vec3(7.0f, 4.5f, -8.0f)));
}

void Lights::setLightSourceToShader(const ShaderProgram &shader, bool to_depth) {
    if (!to_depth) {
//        shader.SetUniform("lightPos", lightPos);
        for (std::size_t i = 0; i < directionalLights.size(); ++i)
            directionalLights[i].setToShader(shader, i);
        for (std::size_t i = 0; i < pointLights.size(); ++i)
            pointLights[i].setToShader(shader, i);
    }
    shader.SetUniform("lightSpaceMatrix", lightSpaceMatrix);
}
