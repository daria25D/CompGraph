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


void KeyMouseControls::mouseCallback(GLFWwindow *window, double x_pos, double y_pos) {
    getInstance().setMouseCallback(window, x_pos, y_pos);
}

void KeyMouseControls::setMouseCallback(GLFWwindow *window, double x_pos, double y_pos) {
    float xOffset = x_pos - lastX;
    float yOffset = y_pos - lastY;
    lastX = x_pos;
    lastY = y_pos;

    xOffset *= MOUSE_SENSITIVITY;
    yOffset *= MOUSE_SENSITIVITY;

    get_camera()->processCameraRotationOnMouse(xOffset, yOffset);
}

void KeyMouseControls::processActionKeys(bool &z_test) {
    if (pressedKeys[GLFW_KEY_2]) {
        z_test = true;
    }
    if (pressedKeys[GLFW_KEY_1]) {
        z_test = false;
    }
    if (pressedKeys[GLFW_KEY_W]) {
        get_camera()->processCameraMovement(FORWARD);
    }
    if (pressedKeys[GLFW_KEY_S]) {
        get_camera()->processCameraMovement(BACKWARD);
    }
    if (pressedKeys[GLFW_KEY_A]) {
        get_camera()->processCameraMovement(LEFT);
    }
    if (pressedKeys[GLFW_KEY_D]) {
        get_camera()->processCameraMovement(RIGHT);
    }
    //TODO add more keys
    /*
     * shift for speed up moving around
     * Q/E for rotating left and right
     * X/Z for rotating up and down
     * OR: Q/E/X/Z can be replaced by mouse movement
     */
}
