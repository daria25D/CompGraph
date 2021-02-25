#pragma once

#include <array>

class GLFWwindow;

class KeyMouseControls {

    KeyMouseControls();
    KeyMouseControls(const KeyMouseControls &);
    void operator=(const KeyMouseControls &);
    std::array<bool, 1024> pressedKeys;

public:
    static KeyMouseControls &getInstance();
    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
    void setKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
    void processActionKeys(bool &z_test);
};