#pragma once

#include <array>

class GLFWwindow;

class KeyMouseControls {
    const float MOUSE_SENSITIVITY = 0.1f;
    //TODO make dependent on window size
    double lastX = 512.0, lastY = 390.0;

    std::array<bool, 1024> pressedKeys;

    KeyMouseControls();
    KeyMouseControls(const KeyMouseControls &);
    void operator=(const KeyMouseControls &);

public:
    static KeyMouseControls &getInstance();
    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
    static void mouseCallback(GLFWwindow *window, double x_pos, double y_pos);
    static void scrollCallback(GLFWwindow *window, double x_offset, double y_offset);

    void setKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
    void setMouseCallback(GLFWwindow *window, double x_pos, double y_pos);
    void setScrollCallback(GLFWwindow *window, double x_offset, double y_offset);

    void processActionKeys(bool &z_test);
};