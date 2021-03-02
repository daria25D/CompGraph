#pragma once

#include <memory>
#include <GLFW/glfw3.h>

#include "ShaderProgram.h"
#include "Object.h"
#include "LightSource.h"

#define PATH_TO_OBJECTS "../models/cup.json"
#define LIGHT_POS glm::vec3(1.0f, 6.5f, 0.7f)

struct DestroyglfwWin{
    void operator()(GLFWwindow* ptr){
        glfwDestroyWindow(ptr);
    }
};

class Renderer {
    //TODO better error handling
    enum STATE {
        SUCCESS = 0,
        ERROR_GLFWINIT,
        ERROR_INITWINDOW,
        ERROR_INITGL
    };
    int SHADOW_WIDTH = 1512, SHADOW_HEIGHT = 1512;

    int width, height;
    ShaderProgram program, depthProgram, quadProgram;
    std::unique_ptr<GLFWwindow, DestroyglfwWin> window;

    GLuint FBO = 0;
    GLuint depthMap;

    vector<Object> allObjects;
    Lights allLights;

    bool zTest = false;

    void setupOpenGLContext() const;
    int initOpenGLWindow();
    void setupShaders();
    void initDepthFrameBuffer();
    int Init();

public:
    Renderer(int w, int h);
    ~Renderer();

    void BeforeRender();
    void RenderToDepth();
    void Render();

    GLFWwindow *getWindow();
    bool &getZTestValue();
};

Renderer *get_renderer();
void init_renderer(int w, int h);
void delete_renderer();