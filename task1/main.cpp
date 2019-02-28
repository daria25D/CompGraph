//internal includes
#include "common.h"
#include "ShaderProgram.h"
#include "LiteMath.h"

//External dependencies
#define GLFW_DLL

#include <GLFW/glfw3.h>
#include <random>

static GLsizei WIDTH = 512, HEIGHT = 512; //размеры окна

using namespace LiteMath;

float3 camera_position(0, 0, 5);
float cam_rot[2] = {0, 0};
float mx = float(WIDTH), my = HEIGHT/2.0;
float3 camera_movement(0.0, 0.0, 0.0);
bool keys[350];

void windowResize(GLFWwindow *window, int width, int height) {
    WIDTH = width;
    HEIGHT = height;
    //mx = width;
    //my = height/2.0;
}

static void mouseMove(GLFWwindow *window, double xpos, double ypos) {
    GLfloat mouse_sensitivity  = 37.0f;

    float horiz_movement = float(xpos * 0.15f - mx);
    float vert_movement  = float(ypos * 0.15f - my);

    cam_rot[0] -= vert_movement / mouse_sensitivity;
    cam_rot[1] += horiz_movement / mouse_sensitivity;

    // Limit loking up
    if (cam_rot[0] < -90.0f) {
        cam_rot[0] = -90.0f;
    }
    // Limit looking down
    if (cam_rot[0] > 90.0f) {
        cam_rot[0] = 90.0f;
    }
    // Looking left and right. Limit range from -180.0f to 180.0f
    if (cam_rot[1] < -180.0f) {
        cam_rot[1] += 360.0f;
    }
    if (cam_rot[1] > 180.0f) {
        cam_rot[1] -= 360.0f;
    }
    mx = float(xpos * 0.15f);
    my = float(ypos * 0.15f);

}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (keys[GLFW_KEY_ESCAPE]) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (action == GLFW_PRESS)
        keys[key] = true;
    else if (action == GLFW_RELEASE)
        keys[key] = false;

}

float to_rads(const float &angle) {
    return angle * DEG_TO_RAD;
}

void move_camera() {
    // Camera controls
    //float3 camera_movement(0.0, 0.0, 0.0);

    GLfloat camera_speed = 0.25f;
    if (keys[GLFW_KEY_LEFT_SHIFT])
        camera_speed *= 2;
    if (!keys[GLFW_KEY_LEFT_SHIFT])
        camera_speed = 0.25f;
    if (keys[GLFW_KEY_W]) {
        camera_movement.x = (camera_speed * sin(to_rads(cam_rot[0]))) * cos(to_rads(cam_rot[1]));
        camera_movement.z = (camera_speed * cos(to_rads(cam_rot[0])) * -1.0f ) * cos(to_rads(cam_rot[1]));
    }
    if (keys[GLFW_KEY_S]) {
        camera_movement.x = (camera_speed * sin(to_rads(cam_rot[0])) * -1.0f) * cos(to_rads(cam_rot[1]));
        camera_movement.z = (camera_speed * cos(to_rads(cam_rot[0]))) * cos(to_rads(cam_rot[1]));
    }
    if (keys[GLFW_KEY_A]) {
        camera_movement.x = -camera_speed * cos(to_rads(cam_rot[1]));
        camera_movement.z = -camera_speed * sin(to_rads(cam_rot[1]));
    }
    if (keys[GLFW_KEY_D]) {
        camera_movement.x = camera_speed * cos(to_rads(cam_rot[1]));
        camera_movement.z = camera_speed * sin(to_rads(cam_rot[1]));
    }
    if (!keys[GLFW_KEY_W] && !keys[GLFW_KEY_S] && !keys[GLFW_KEY_A] && !keys[GLFW_KEY_D]) {
        camera_movement = float3(0.0, 0.0, 0.0);
    }

    //camera_position += camera_movement;

}

int initGL() {
    int res = 0;
    //грузим функции opengl через glad
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    return 0;
}


int main(int argc, char **argv) {
    if (!glfwInit()) {
        std::cout << "could not init" << std::endl;
        return -1;
    }
    //запрашиваем контекст opengl версии 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    //antialiasing?
    glfwWindowHint(GLFW_SAMPLES, 8);


    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL ray marching sample", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwSetCursorPosCallback(window, mouseMove);
    glfwSetWindowSizeCallback(window, windowResize);

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    glfwSetKeyCallback(window, key_callback);

    if (initGL() != 0)
        return -1;

    //Reset any OpenGL errors which could be present for some reason
    GLenum gl_error = glGetError();
    while (gl_error != GL_NO_ERROR)
        gl_error = glGetError();

    //создание шейдерной программы из двух файлов с исходниками шейдеров
    //используется класс-обертка ShaderProgram
    std::unordered_map<GLenum, std::string> shaders;
    shaders[GL_VERTEX_SHADER] = "vertex.glsl";
    shaders[GL_FRAGMENT_SHADER] = "fragment.glsl";
    ShaderProgram program(shaders);
    GL_CHECK_ERRORS;

    glfwSwapInterval(1); // force 60 frames per second
    //antialiasing?
    glEnable(GL_MULTISAMPLE);
    //Создаем и загружаем геометрию поверхности
    //
    int width, height;
    //unsigned char * image = SOIL_load_image("../textures/container.jpg", &width, &height, 0, SOIL_LOAD_RGB);
    GLuint g_vertexBufferObject;
    GLuint g_vertexArrayObject;
    {
        float quadPos[] =
                {
                        -1.0f, 1.0f,    // v0 - top left corner
                        -1.0f, -1.0f,    // v1 - bottom left corner
                        1.0f, 1.0f,    // v2 - top right corner
                        1.0f, -1.0f      // v3 - bottom right corner
                };

        g_vertexBufferObject = 0;
        GLuint vertexLocation = 0; // simple layout, assume have only positions at location = 0

        glGenBuffers(1, &g_vertexBufferObject);
        GL_CHECK_ERRORS;
        glBindBuffer(GL_ARRAY_BUFFER, g_vertexBufferObject);
        GL_CHECK_ERRORS;
        glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), (GLfloat *) quadPos, GL_STATIC_DRAW);
        GL_CHECK_ERRORS;

        glGenVertexArrays(1, &g_vertexArrayObject);
        GL_CHECK_ERRORS;
        glBindVertexArray(g_vertexArrayObject);
        GL_CHECK_ERRORS;

        glBindBuffer(GL_ARRAY_BUFFER, g_vertexBufferObject);
        GL_CHECK_ERRORS;
        glEnableVertexAttribArray(vertexLocation);
        GL_CHECK_ERRORS;
        glVertexAttribPointer(vertexLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
        GL_CHECK_ERRORS;

        glBindVertexArray(0);
    }

    //цикл обработки сообщений и отрисовки сцены каждый кадр
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        move_camera();
        //очищаем экран каждый кадр
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        GL_CHECK_ERRORS;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GL_CHECK_ERRORS;
        program.StartUseShader();
        GL_CHECK_ERRORS;
        //std::cout << camera_position.x << " " << camera_position.y << " " << camera_position.z << std::endl;
        float4x4 camRotMatrix = mul(rotate_Y_4x4(-cam_rot[1]), rotate_X_4x4(+cam_rot[0]));
        float4x4 camTransMatrix = translate4x4(camera_position);
        float4x4 rayMatrix = mul(camTransMatrix, camRotMatrix);
        camera_position += mul(camRotMatrix, camera_movement);
        //float4x4 matr = mul(camRotMatrix, camTransMatrix);
        //rayMatrix = mul(matr, rayMatrix);
        //rayMatrix = mul(rayMatrix, camRotMatrix);
        //отслеживать нажата ли кнопка или отпущена, матрицу изменять в цикле
        //матрицу  сдвига умножать на матрицу сдвига и на матрицу поворота текущую слева
        //program.SetUniform("radius", 0.5f);
        program.SetUniform("g_rayMatrix", rayMatrix);
        //float cam_pos[3] = {camera_position.x, camera_position.y, camera_position.z};
        //program.SetUniform("cam_pos", cam_pos);
        GL_CHECK_ERRORS;
        program.SetUniform("g_screenWidth", WIDTH);
        GL_CHECK_ERRORS;
        program.SetUniform("g_screenHeight", HEIGHT);
        GL_CHECK_ERRORS;
        // очистка и заполнение экрана цветом
        //
        glViewport(0, 0, WIDTH, HEIGHT);
        GL_CHECK_ERRORS;
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        GL_CHECK_ERRORS;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        GL_CHECK_ERRORS;

        // draw call
        //
//        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(g_vertexArrayObject); GL_CHECK_ERRORS;
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);  GL_CHECK_ERRORS;
        GL_CHECK_ERRORS;  // The last parameter of glDrawArrays is equal to VS invocations
        program.StopUseShader();

        glfwSwapBuffers(window);
    }

    //очищаем vbo и vao перед закрытием программы
    //
    glDeleteVertexArrays(1, &g_vertexArrayObject);
    glDeleteBuffers(1, &g_vertexBufferObject);

//    SOIL_free_image_data(image);
//    glBindTexture(GL_TEXTURE_2D, 0);

    glfwTerminate();
    return 0;
}
