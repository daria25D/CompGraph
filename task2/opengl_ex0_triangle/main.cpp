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

static const GLsizei WIDTH = 640, HEIGHT = 480; //размеры окна

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

bool loadOBJ(
       const char * path,
        vector < glm::vec3 > & out_vertices,
        vector < glm::vec2 > & out_uvs,
        vector < glm::vec3 > & out_normals
) {
    vector< unsigned int > vertexIndices, uvIndices, normalIndices;
    vector< glm::vec3 > temp_vertices;
    vector< glm::vec2 > temp_uvs;
    vector< glm::vec3 > temp_normals;

    FILE * file = fopen(path, "r");
    if( file == NULL ){
        printf("Impossible to open the file !\n");
        return false;
    }

    while( true ) {
        char lineHeader[128];
        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break;
        if (strcmp(lineHeader, "v") == 0) {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
        } else if (strcmp(lineHeader, "vt") == 0) {
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y);
            temp_uvs.push_back(uv);
        } else if (strcmp(lineHeader, "vn") == 0) {
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        } else if (strcmp(lineHeader, "f") == 0) {
            string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0],
                                 &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2],
                                 &normalIndex[2]);
            if (matches != 9) {
                printf("File can't be read by our simple parser : ( Try exporting with other options\n");
                return false;
            }
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            uvIndices.push_back(uvIndex[0]);
            uvIndices.push_back(uvIndex[1]);
            uvIndices.push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        }
    }
    for (int i = 0; i < vertexIndices.size(); i++) {
        unsigned int vertex_index = vertexIndices[i];
        glm::vec3 vertex = temp_vertices[vertex_index - 1];
        out_vertices.push_back(vertex);
    }
    for (int i = 0; i < uvIndices.size(); i++) {
        unsigned int uv_index = uvIndices[i];
        glm::vec2 uv = temp_uvs[uv_index - 1];
        out_uvs.push_back(uv);
    }
    for (int i = 0; i < normalIndices.size(); i++) {
        unsigned int normal_index = normalIndices[i];
        glm::vec3 normal = temp_normals[normal_index - 1];
        out_vertices.push_back(normal);
    }
    return true;
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
    shaders[GL_VERTEX_SHADER] = "vertex.glsl";
    shaders[GL_FRAGMENT_SHADER] = "fragment.glsl";
    ShaderProgram program(shaders);
    GL_CHECK_ERRORS;

    glfwSwapInterval(1); // force 60 frames per second

    //matrices of view
    glm::mat4 proj = glm::perspective(45.0f, (float)WIDTH/(float)HEIGHT, 0.1f, 100.0f);
    glm::mat4 model(1.0f);
    model = glm::rotate(model, 10.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 view(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));


    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CW);

    Model model1("../objects/cup.obj");
    //Создаем и загружаем геометрию поверхности
    //

//    vector< glm::vec3 > vertices;
//    vector< glm::vec2 > uvs;
//    vector< glm::vec3 > normals; // Won't be used at the moment.
//    bool res = loadOBJ("../objects/sphere.obj", vertices, uvs, normals);
//
//    GLuint g_vertexArrayObject = 0; //only one needed
//    GLuint g_vertexBufferObject;
//    GLuint g_uvBuffer; //add for normals
//    unsigned char * image;
//    GLuint texture;
//    {
//        g_vertexBufferObject = 0;
//        g_uvBuffer = 0;
//        GLuint vertexLocation = 0; // simple layout, assume have only positions at location = 0
//        GLuint uvLocation = 1; //add for normals layout = 2
//        //TODO: add uv buffer
//        glGenBuffers(1, &g_vertexBufferObject);
//        GL_CHECK_ERRORS;
//        glBindBuffer(GL_ARRAY_BUFFER, g_vertexBufferObject);
//        GL_CHECK_ERRORS;
//        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
//
//        glGenBuffers(1, &g_uvBuffer);
//        glBindBuffer(GL_ARRAY_BUFFER, g_uvBuffer);
//        glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
//        GL_CHECK_ERRORS;
//
//        //add 3 lines for normals
//
//        glGenVertexArrays(1, &g_vertexArrayObject);
//        GL_CHECK_ERRORS;
//        glBindVertexArray(g_vertexArrayObject);
//        GL_CHECK_ERRORS;
//
//        glBindBuffer(GL_ARRAY_BUFFER, g_vertexBufferObject);
//        GL_CHECK_ERRORS;
//        glEnableVertexAttribArray(vertexLocation);
//        GL_CHECK_ERRORS;
//        glVertexAttribPointer(vertexLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
//
//        glBindBuffer(GL_ARRAY_BUFFER, g_uvBuffer);
//        glEnableVertexAttribArray(uvLocation);
//        glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
//
//        //add 3 lines for normals
//
//        //texture binding
//        int width, height;
//        image = SOIL_load_image("../textures/marble.jpg", &width, &height, 0, SOIL_LOAD_RGB);
//        glGenTextures(1, &texture);
//        glBindTexture(GL_TEXTURE_2D, texture);
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//        glGenerateMipmap(GL_TEXTURE_2D);
//
//        GL_CHECK_ERRORS;
//        glBindVertexArray(0);
//        //glBindVertexArray(1);
//    }

    //цикл обработки сообщений и отрисовки сцены каждый кадр
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

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

        GL_CHECK_ERRORS;

        // очистка и заполнение экрана цветом
        //
        glViewport(0, 0, WIDTH, HEIGHT);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // draw call
//        //
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, texture);
//        glBindVertexArray(g_vertexArrayObject);
//        GL_CHECK_ERRORS;
//        glDrawArrays(GL_TRIANGLES, 0, 3 * vertices.size());
        model1.Draw(program);
        GL_CHECK_ERRORS;  // The last parameter of glDrawArrays is equal to VS invocations


        program.StopUseShader();

        glfwSwapBuffers(window);
    }

    //очищаем vboи vao перед закрытием программы
//    //
//    glDeleteVertexArrays(1, &g_vertexArrayObject);
//    glDeleteBuffers(1, &g_vertexBufferObject);
//    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);

    glfwTerminate();
    return 0;
}
