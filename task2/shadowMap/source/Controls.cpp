#include <GLFW/glfw3.h>
#include "Controls.h"
#include "Camera.h"

KeyMouseControls::KeyMouseControls() {
    pressedKeys.fill(false);
}

KeyMouseControls &KeyMouseControls::getInstance() {
    static KeyMouseControls instance;
    return instance;
}

void KeyMouseControls::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode) {
    getInstance().setKeyCallback(window, key, scancode, action, mode);
}

void KeyMouseControls::setKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode) {
    if (action == GLFW_PRESS) {
        pressedKeys[key] = true;
    } else if (action == GLFW_RELEASE) {
        pressedKeys[key] = false;
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

void KeyMouseControls::processActionKeys(bool &z_test) {
    if (pressedKeys[GLFW_KEY_2]) {
        z_test = true;
    }
    if (pressedKeys[GLFW_KEY_1]) {
        z_test = false;
    }
    if (pressedKeys[GLFW_KEY_W]) {
        get_camera()->moveCameraPosition(get_camera()->getCameraSpeed() *
                                         get_camera()->getCameraFront());
    }
    if (pressedKeys[GLFW_KEY_S]) {
        get_camera()->moveCameraPosition(-get_camera()->getCameraSpeed() *
                                          get_camera()->getCameraFront());
    }
    if (pressedKeys[GLFW_KEY_A]) {
        get_camera()->moveCameraPosition(
                -get_camera()->getCameraSpeed() *
                glm::normalize(
                        glm::cross(get_camera()->getCameraFront(),
                                   get_camera()->getCameraUp())
                )
        );
    }
    if (pressedKeys[GLFW_KEY_D]) {
        get_camera()->moveCameraPosition(
                get_camera()->getCameraSpeed() *
                glm::normalize(
                        glm::cross(get_camera()->getCameraFront(),
                                   get_camera()->getCameraUp())
                )
        );
    }
    //TODO add more keys
}
