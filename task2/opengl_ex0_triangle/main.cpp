//internal includes
#include "common.h"
#include "ShaderProgram.h"

//External dependencies
#define GLFW_DLL

#include <GLFW/glfw3.h>
#include <random>
#include <string.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

#include <SOIL/SOIL.h>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags


#include "Mesh.h"
#include "Model.h"

using namespace std;

static const GLsizei WIDTH = 1024, HEIGHT = 780; //размеры окна
//bool z_test = true;
bool pressedKeys[1024];

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (action == GLFW_PRESS) {
        pressedKeys[key] = true;
    } else if (action == GLFW_RELEASE) {
        pressedKeys[key] = false;
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}
void processActionKeys() {
    if (pressedKeys[GLFW_KEY_2]) {
        z_test = true;
    }
    if (pressedKeys[GLFW_KEY_1]) {
        z_test = false;
    }
}

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

int main(int argc, char **argv) {
    if (!glfwInit())
        return -1;

    //запрашиваем контекст opengl версии 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL basic sample", nullptr, nullptr);
    glfwSetKeyCallback(window, key_callback);
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

    //создание шейдерной программы из двух файлов с исходниками шейдеров
    //используется класс-обертка ShaderProgram
    unordered_map<GLenum, string> shaders;
    if (!z_test) {
        shaders[GL_VERTEX_SHADER] = "vertex.glsl";
        shaders[GL_FRAGMENT_SHADER] = "fragment.glsl";
    } else {
        shaders[GL_VERTEX_SHADER] = "z_test_vert.glsl";
        shaders[GL_FRAGMENT_SHADER] = "z_test_frag.glsl";
    }
    ShaderProgram program(shaders);
    GL_CHECK_ERRORS;
    glfwSwapInterval(1); // force 60 frames per second

    //matrices of view
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)WIDTH/(float)HEIGHT, 0.1f, 120.0f);
    glm::mat4 model(1.0f);
    model = glm::rotate(model, glm::radians(35.0f), glm::vec3(1.0f, 0.5f, .0f));
    glm::mat4 view(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -6.0f));


    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CW);

    Model model1("../objects/fabric.obj", "fabric");
    Model model2("../objects/cup.obj", "cup");
    Model model3("../objects/bowl.obj", "bowl");
    Model model4("../objects/plane.obj", "plane");

    //цикл обработки сообщений и отрисовки сцены каждый кадр
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        processActionKeys();

        if (!z_test) {
            shaders[GL_VERTEX_SHADER] = "vertex.glsl";
            shaders[GL_FRAGMENT_SHADER] = "fragment.glsl";
        } else {
            shaders[GL_VERTEX_SHADER] = "z_test_vert.glsl";
            shaders[GL_FRAGMENT_SHADER] = "z_test_frag.glsl";
        }
        ShaderProgram program(shaders);

        //очищаем экран каждый кадр
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        GL_CHECK_ERRORS;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GL_CHECK_ERRORS;

        program.StartUseShader();

        GLint modelLoc = glGetUniformLocation(program.GetProgram(), "model");
        GL_CHECK_ERRORS;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        GL_CHECK_ERRORS;

        GLint projLoc = glGetUniformLocation(program.GetProgram(), "proj");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
        GL_CHECK_ERRORS;

        GLint viewLoc = glGetUniformLocation(program.GetProgram(), "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        GL_CHECK_ERRORS;

        // очистка и заполнение экрана цветом
        //
        glViewport(0, 0, WIDTH, HEIGHT);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // draw call
        model1.Draw(program);
        model2.Draw(program);
        model3.Draw(program);
        model4.Draw(program);
        GL_CHECK_ERRORS;


        program.StopUseShader();
        //glDrawBuffer(GL_NONE);

        glfwSwapBuffers(window);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    glfwTerminate();
    return 0;
}
