#include <string>
#include <unordered_map>
#include "common.h"

#include "Camera.h"
#include "Render.h"

using namespace std;

static unsigned int quadVAO = 0;
static unsigned int quadVBO;

static unique_ptr<Renderer> renderer;

Renderer *get_renderer() {
    return renderer.get();
}

void init_renderer(int w, int h) {
    renderer = make_unique<Renderer>(w, h);
}

void delete_renderer() {
    renderer.reset();
}

static void renderQuad() {
    if (quadVAO == 0) {
        float quadVertices[] = {
                -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

static int initGL() {
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        cout << "Failed to initialize OpenGL context" << endl;
        return -1;
    }

    cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
    cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
    cout << "Version: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

    return 0;
}

Renderer::Renderer(int w, int h) : width(w), height(h), light(w, h, LIGHT_POS)
{
//    window.reset();
    int initResult = Init();
    //TODO process initResult
    if (initResult != SUCCESS) {
        cout << "Error appeared, terminating...";
        throw runtime_error("Error");
    }
    zTest = false;
}

void Renderer::setupOpenGLContext() const {
    //OpenGL version 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4);
}

int Renderer::initOpenGLWindow() {
    window.reset(glfwCreateWindow(width, height, "OpenGL", nullptr, nullptr));
    if (window == nullptr) {
//        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return ERROR_INITWINDOW;
    }
    glfwMakeContextCurrent(window.get());
    return SUCCESS;
}

void Renderer::setupShaders() {
    unordered_map<GLenum, string> shaders;
    unordered_map<GLenum, string> shadersDepth;
    unordered_map<GLenum, string> shadersPass;
    shaders[GL_VERTEX_SHADER] = "vertex.glsl";
    shaders[GL_FRAGMENT_SHADER] = "fragment.glsl";

    shadersDepth[GL_VERTEX_SHADER] = "z_test_vert.glsl";
    shadersDepth[GL_FRAGMENT_SHADER] = "z_test_frag.glsl";

    shadersPass[GL_VERTEX_SHADER] = "pass_vert.glsl";
    shadersPass[GL_FRAGMENT_SHADER] = "pass_frag.glsl";

    program = ShaderProgram(shaders);
    depthProgram = ShaderProgram(shadersDepth);
    quadProgram = ShaderProgram(shadersPass);
}

void Renderer::initDepthFrameBuffer() {
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GL_CHECK_ERRORS;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL_CHECK_ERRORS;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CHECK_ERRORS;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);

    GL_CHECK_ERRORS;

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("GL_FRAMEBUFFER_COMPLETE failed, CANNOT use FBO\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GL_CHECK_ERRORS;
}

int Renderer::Init()
{
    if (!glfwInit())
        return ERROR_GLFWINIT;

    setupOpenGLContext();
    initOpenGLWindow();


    if (initGL() != 0)
        return ERROR_INITGL;

    //Reset any OpenGL errors which could be present for some reason
    GLenum gl_error = glGetError();
    while (gl_error != GL_NO_ERROR)
        gl_error = glGetError();

    glfwSwapInterval(1); // force 60 frames per second

    setupShaders();
    allObjects = setup_objects_from_json(PATH_TO_OBJECTS);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CW);
    glEnable(GL_MULTISAMPLE);

    initDepthFrameBuffer();
    allLights.directionalLights.push_back(DirectionalLight());

    return SUCCESS;
}

void Renderer::BeforeRender() {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    GL_CHECK_ERRORS;
    //is it necessary?
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    GL_CHECK_ERRORS;
}

void Renderer::RenderToDepth() {
    depthProgram.StartUseShader();
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    light.setLightSourceToShader(depthProgram, true);

    for (auto &object : allObjects)
        object.DrawToDepth(depthProgram);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glCullFace(GL_FRONT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
}

void Renderer::Render() {
    program.StartUseShader();

    glViewport(0, 0, width, height);
    GL_CHECK_ERRORS;

    light.setLightSourceToShader(program);
    for (auto & dl : allLights.directionalLights)
        dl.setToShader(program);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    program.SetUniform("shadowMap", 4);
    program.SetUniform("viewPos", get_camera()->getCameraPosition());
    GL_CHECK_ERRORS;
    for (auto &object : allObjects)
        object.Draw(program, get_camera()->getProjMatrix(), get_camera()->getViewMatrix());
    if (zTest) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        quadProgram.StartUseShader();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        program.SetUniform("shadowMap", 0);
        renderQuad();
    }
    glfwSwapBuffers(window.get());
}

GLFWwindow *Renderer::getWindow() {
    return window.get();
}

bool &Renderer::getZTestValue() {
    return zTest;
}

Renderer::~Renderer() {
    program.Release();
    depthProgram.Release();
    quadProgram.Release();
    window.reset();
}
