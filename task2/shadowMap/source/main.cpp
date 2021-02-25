//External dependencies
#define GLFW_DLL

#include <chrono>
#include <glm/glm.hpp>

//internal includes
#include "common.h"
#include "Controls.h"
#include "Camera.h"
#include <GLFW/glfw3.h>
#include "Render.h"

using namespace std;

static const GLsizei WIDTH = 1024, HEIGHT = 780; //размеры окна

int main(int argc, char **argv) {
    //camera setup
    glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
    init_camera(cameraPos, cameraFront, cameraUp, WIDTH, HEIGHT);

    init_renderer(WIDTH, HEIGHT);
    KeyMouseControls &controls = KeyMouseControls::getInstance();

    //controls callback setup
    glfwSetInputMode(get_renderer()->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetKeyCallback(get_renderer()->getWindow(), &KeyMouseControls::keyCallback);
    glfwSetCursorPosCallback(get_renderer()->getWindow(), &KeyMouseControls::mouseCallback);
    glfwSetScrollCallback(get_renderer()->getWindow(), &KeyMouseControls::scrollCallback);

//    auto t_start = chrono::high_resolution_clock::now();
    while (!glfwWindowShouldClose(get_renderer()->getWindow())) {
        glfwPollEvents();
        controls.processActionKeys(get_renderer()->getZTestValue());

        //TODO separate time management from camera?
        get_camera()->updateCurrentTime();

        get_renderer()->BeforeRender();
        get_renderer()->RenderToDepth();
        get_renderer()->Render();

//        t_now = chrono::high_resolution_clock::now();
//        time = chrono::duration_cast<::chrono::duration<float>>(t_now - t_start).count();
//        model = glm::translate(model, glm::vec3(-1.4f, 0.0f, 0.764f));
//        model = glm::rotate(model, 0.5f * time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    delete_camera();
    delete_renderer();
    glfwTerminate();
    return 0;
}
