//internal includes
#include "common.h"
#include "ShaderProgram.h"
#include "Mesh.h"
#include "Model.h"
#include "Controls.h"
#include "Object.h"
#include "Camera.h"

//External dependencies
#define GLFW_DLL

#include <GLFW/glfw3.h>
#include <random>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

using namespace std;

bool z_test = false;

static const GLsizei WIDTH = 1024, HEIGHT = 780; //размеры окна


int initGL() {
    int res = 0;
    //грузим функции opengl через glad
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


unsigned int quadVAO = 0;
unsigned int quadVBO;

void renderQuad() {
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

int main(int argc, char **argv) {
    if (!glfwInit())
        return -1;
    KeyMouseControls &controls = KeyMouseControls::getInstance();

    //запрашиваем контекст opengl версии 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL basic sample", nullptr, nullptr);
    glfwSetKeyCallback(window, &KeyMouseControls::keyCallback);
    if (window == nullptr) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (initGL() != 0)
        return -1;

    //Reset any OpenGL errors which could be present for some reason
    GLenum gl_error = glGetError();
    while (gl_error != GL_NO_ERROR)
        gl_error = glGetError();

    //используется класс-обертка ShaderProgram
    unordered_map<GLenum, string> shaders;
    unordered_map<GLenum, string> shadersDepth;
    unordered_map<GLenum, string> shadersPass;

    shaders[GL_VERTEX_SHADER] = "vertex.glsl";
    shaders[GL_FRAGMENT_SHADER] = "fragment.glsl";

    shadersDepth[GL_VERTEX_SHADER] = "z_test_vert.glsl";
    shadersDepth[GL_FRAGMENT_SHADER] = "z_test_frag.glsl";

    shadersPass[GL_VERTEX_SHADER] = "pass_vert.glsl";
    shadersPass[GL_FRAGMENT_SHADER] = "pass_frag.glsl";

    ShaderProgram program(shaders);
    ShaderProgram depth_prog(shadersDepth);
    ShaderProgram quad_prog(shadersPass);

    vector<Object> allObjects = setup_objects_from_json("../models/models.json");

    GL_CHECK_ERRORS;
    glfwSwapInterval(1); // force 60 frames per second

    glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
    init_camera(cameraPos, cameraFront, cameraUp);
    //matrices of view
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float) WIDTH / (float) HEIGHT, 0.1f, 100.0f);
    glm::mat4 view(1.0f);
    cameraPos = get_camera()->getCameraPosition();
    view = glm::lookAt(cameraPos, cameraPos + get_camera()->getCameraFront(), get_camera()->getCameraUp());

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CW);
    glEnable(GL_MULTISAMPLE);

    GLuint FBO = 0;
    GLuint depthMap;
    int SHADOW_WIDTH = 1512, SHADOW_HEIGHT = 1512;
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


    auto t_start = chrono::high_resolution_clock::now();
    glm::vec3 lightPos(1.0f, 6.5f, 0.7f);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        controls.processActionKeys(z_test);

        //TODO separate time management from camera?
        get_camera()->updateCurrentTime();
        //TODO store view matrix inside camera class? recalculate view on change?
        cameraPos = get_camera()->getCameraPosition();
        view = glm::lookAt(cameraPos, cameraPos + get_camera()->getCameraFront(), get_camera()->getCameraUp());

        GL_CHECK_ERRORS;
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        GL_CHECK_ERRORS;

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GL_CHECK_ERRORS;

        depth_prog.StartUseShader();
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        lightProjection = glm::perspective(glm::radians(45.0f), (float) WIDTH / (float) HEIGHT, 0.1f, 100.0f);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;

        glUniformMatrix4fv(glGetUniformLocation(depth_prog.GetProgram(), "lightSpaceMatrix"), 1, GL_FALSE,
                           &lightSpaceMatrix[0][0]);
        GL_CHECK_ERRORS;

        for (auto &object : allObjects)
            object.DrawToDepth(depth_prog);
        auto t_now = chrono::high_resolution_clock::now();
        float time = chrono::duration_cast<::chrono::duration<float>>(t_now - t_start).count();
//        model = glm::translate(model, glm::vec3(-1.4f, 0.0f, 0.764f));
//        model = glm::rotate(model, 0.5f * time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        GL_CHECK_ERRORS;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glCullFace(GL_FRONT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.5f, 1.0f);

        program.StartUseShader();

        glViewport(0, 0, WIDTH, HEIGHT);
        GL_CHECK_ERRORS;
        GL_CHECK_ERRORS;
        glUniform3fv(glGetUniformLocation(program.GetProgram(), "lightPos"), 1, &lightPos[0]);
        GL_CHECK_ERRORS;
        glUniformMatrix4fv(glGetUniformLocation(program.GetProgram(), "lightSpaceMatrix"), 1, GL_FALSE,
                           &lightSpaceMatrix[0][0]);
        GL_CHECK_ERRORS;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glUniform1i(glGetUniformLocation(program.GetProgram(), "shadowMap"), 4);
        GL_CHECK_ERRORS;

        for (auto &object : allObjects)
            object.Draw(program, proj, view);

        t_now = chrono::high_resolution_clock::now();
        time = chrono::duration_cast<::chrono::duration<float>>(t_now - t_start).count();
//        model = glm::translate(model, glm::vec3(-1.4f, 0.0f, 0.764f));
//        model = glm::rotate(model, 0.5f * time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        if (z_test) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
            glViewport(0, 0, WIDTH, HEIGHT);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            quad_prog.StartUseShader();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, depthMap);
            GLuint texID = glGetUniformLocation(quad_prog.GetProgram(), "shadowMap");
            glUniform1i(texID, 0);
            renderQuad();
        }
        glfwSwapBuffers(window);
    }
    delete_camera();
    glfwTerminate();
    return 0;
}
