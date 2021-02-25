#include <glm/gtc/matrix_transform.hpp>

#include "LightSource.h"
#include "ShaderProgram.h"

LightSource::LightSource(int w, int h, glm::vec3 light_pos) :
    width(w), height(h), lightPos(light_pos)
{
    lightProjection = glm::perspective(glm::radians(45.0f), (float) width / (float) height, 0.1f, 100.0f);
    lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;
}

void LightSource::setLightSourceToShader(const ShaderProgram &shader) {

    glUniform3fv(glGetUniformLocation(shader.GetProgram(), "lightPos"), 1, &lightPos[0]);
    GL_CHECK_ERRORS;
    glUniformMatrix4fv(glGetUniformLocation(shader.GetProgram(), "lightSpaceMatrix"), 1, GL_FALSE,
                       &lightSpaceMatrix[0][0]);
}