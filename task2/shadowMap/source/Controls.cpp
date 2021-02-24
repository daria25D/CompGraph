#include "Controls.h"
#include <GLFW/glfw3.h>

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
    //TODO add more keys
}
