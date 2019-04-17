//internal includes
#include "common.h"
#include "ShaderProgram.h"

//External dependencies
#define GLFW_DLL

#include <GLFW/glfw3.h>
#include <random>
#include <string.h>
#include <chrono>

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

bool z_test = false;

static const GLsizei WIDTH = 1024, HEIGHT = 780; //размеры окна
//bool z_test = true;
bool pressedKeys[1024];

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
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

vector<Texture> setup_texture(string type) {
    vector<Texture> textures;
    if (type == "fabric") {
        Texture texture;
        texture.id = TextureFromFile("fabric_diffuse.dds", "../textures");
        texture.type = "texture_diffuse";
        texture.path = "fabric_diffuse.dds";
        textures.push_back(texture);
        texture.id = TextureFromFile("fabric_normal.dds", "../textures");
        texture.type = "texture_normal";
        texture.path = "fabric_normal.dds";
        textures.push_back(texture);
        texture.id = TextureFromFile("fabric_height.dds", "../textures");
        texture.type = "texture_height";
        texture.path = "fabric_height.dds";
        textures.push_back(texture);
    } else if (type == "cup") {
        Texture texture;
        texture.id = TextureFromFile("quartz_diffuse.dds", "../textures");
        texture.type = "texture_diffuse";
        texture.path = "quartz_diffuse.dds";
        textures.push_back(texture);
        texture.id = TextureFromFile("quartz_normal.dds", "../textures");
        texture.type = "texture_normal";
        texture.path = "quartz_normal.dds";
        textures.push_back(texture);
        texture.id = TextureFromFile("quartz_height.dds", "../textures");
        texture.type = "texture_height";
        texture.path = "quartz_height.dds";
        textures.push_back(texture);
    } else if (type == "bowl") {
        Texture texture;
        texture.id = TextureFromFile("quartz_diffuse.dds", "../textures");
        texture.type = "texture_diffuse";
        texture.path = "quartz_diffuse.dds";
        textures.push_back(texture);
        texture.id = TextureFromFile("quartz_normal.dds", "../textures");
        texture.type = "texture_normal";
        texture.path = "quartz_normal.dds";
        textures.push_back(texture);
        texture.id = TextureFromFile("quartz_height.dds", "../textures");
        texture.type = "texture_height";
        texture.path = "quartz_height.dds";
        textures.push_back(texture);
    } else if (type == "plane") {
        Texture texture;
        texture.id = TextureFromFile("wood_diffuse.dds", "../textures");
        texture.type = "texture_diffuse";
        texture.path = "wood_diffuse.dds";
        textures.push_back(texture);
        texture.id = TextureFromFile("wood_normal.dds", "../textures");
        texture.type = "texture_normal";
        texture.path = "wood_normal.dds";
        textures.push_back(texture);
        texture.id = TextureFromFile("wood_height.dds", "../textures");
        texture.type = "texture_height";
        texture.path = "wood_height.dds";
        textures.push_back(texture);
    } else if (type == "sponge") {
        Texture texture;
        texture.id = TextureFromFile("sponge_diffuse.dds", "../textures");
        texture.type = "texture_diffuse";
        texture.path = "sponge_diffuse.dds";
        textures.push_back(texture);
        texture.id = TextureFromFile("sponge_normal.dds", "../textures");
        texture.type = "texture_normal";
        texture.path = "sponge_normal.dds";
        textures.push_back(texture);
        texture.id = TextureFromFile("sponge_height.dds", "../textures");
        texture.type = "texture_height";
        texture.path = "sponge_height.dds";
        textures.push_back(texture);
    }
    return textures;
}

void upload_texture(ShaderProgram shader, vector<Texture> textures) {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;
    for (unsigned int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
        // retrieve texture number (the N in diffuse_textureN)
        string number;
        string name = textures[i].type;
        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++); // transfer unsigned int to stream
        else if (name == "texture_normal")
            number = std::to_string(normalNr++); // transfer unsigned int to stream
        else if (name == "texture_height")
            number = std::to_string(heightNr++); // transfer unsigned int to stream

        // now set the sampler to the correct texture unit
        glUniform1i(glGetUniformLocation(shader.GetProgram(), (name + number).c_str()), i);
        // and finally bind the texture
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

int main(int argc, char **argv) {
    if (!glfwInit())
        return -1;

    //запрашиваем контекст opengl версии 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4);

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

    GL_CHECK_ERRORS;
    glfwSwapInterval(1); // force 60 frames per second

    //matrices of view
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float) WIDTH / (float) HEIGHT, 0.1f, 100.0f);
    glm::mat4 model(1.0f);
    model = glm::rotate(model, glm::radians(30.0f), glm::vec3(1.0f, 1.0f, .0f));
    glm::mat4 view(1.0f);
    view = glm::translate(view, glm::vec3(.0f, .0f, -6.0f));

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

    Model model1("../objects/fabric.obj", "fabric");
    vector<Texture> fabric_texture = setup_texture("fabric");
    Model model2("../objects/cup.obj", "cup");
    vector<Texture> cup_texture = setup_texture("cup");
    Model model3("../objects/bowl.obj", "bowl");
    vector<Texture> bowl_texture = setup_texture("bowl");
    Model model4("../objects/plane.obj", "plane");
    vector<Texture> plane_texture = setup_texture("plane");
    Model model5("../objects/sponge.obj", "sponge");
    vector<Texture> sponge_texture = setup_texture("sponge");
    GL_CHECK_ERRORS;

    auto t_start = chrono::high_resolution_clock::now();
    glm::vec3 lightPos(1.0f, 6.5f, 0.7f);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        processActionKeys();
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

        model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(30.0f), glm::vec3(1.0f, 1.0f, .0f));

        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        lightProjection = glm::perspective(glm::radians(45.0f), (float) WIDTH / (float) HEIGHT, 0.1f, 100.0f);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;

        glUniformMatrix4fv(glGetUniformLocation(depth_prog.GetProgram(), "lightSpaceMatrix"), 1, GL_FALSE,
                           &lightSpaceMatrix[0][0]);
        GLint modelLocDepth = glGetUniformLocation(depth_prog.GetProgram(), "model");
        GL_CHECK_ERRORS;
        glUniformMatrix4fv(modelLocDepth, 1, GL_FALSE, glm::value_ptr(model));
        GL_CHECK_ERRORS;

        model1.Draw(depth_prog);
        model2.Draw(depth_prog);
        model3.Draw(depth_prog);
        model4.Draw(depth_prog);
        auto t_now = chrono::high_resolution_clock::now();
        float time = chrono::duration_cast<::chrono::duration<float>>(t_now - t_start).count();
        model = glm::translate(model, glm::vec3(-1.4f, 0.0f, 0.764f));
        model = glm::rotate(model, 0.5f * time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelLocDepth = glGetUniformLocation(depth_prog.GetProgram(), "model");
        GL_CHECK_ERRORS;
        glUniformMatrix4fv(modelLocDepth, 1, GL_FALSE, glm::value_ptr(model));
        GL_CHECK_ERRORS;
        model5.Draw(depth_prog);
        GL_CHECK_ERRORS;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glCullFace(GL_FRONT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glViewport(0, 0, WIDTH, HEIGHT);

        program.StartUseShader();

        glViewport(0, 0, WIDTH, HEIGHT);
        GL_CHECK_ERRORS;
        GL_CHECK_ERRORS;
        glUniform3fv(glGetUniformLocation(program.GetProgram(), "lightPos"), 1, &lightPos[0]);
        GL_CHECK_ERRORS;
        glUniformMatrix4fv(glGetUniformLocation(program.GetProgram(), "lightSpaceMatrix"), 1, GL_FALSE,
                           &lightSpaceMatrix[0][0]);
        model = glm::mat4(1.0);
        model = glm::rotate(model, glm::radians(30.0f), glm::vec3(1.0f, 1.0f, .0f));
        GL_CHECK_ERRORS;
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

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE4);
        GL_CHECK_ERRORS;
        glBindTexture(GL_TEXTURE_2D, depthMap);
        GL_CHECK_ERRORS;
        glUniform1i(glGetUniformLocation(program.GetProgram(), "shadowMap"), 4);

        GL_CHECK_ERRORS;

        upload_texture(program, fabric_texture);
        model1.Draw(program);
        upload_texture(program, cup_texture);
        model2.Draw(program);
        upload_texture(program, bowl_texture);
        model3.Draw(program);
        upload_texture(program, plane_texture);
        model4.Draw(program);
        t_now = chrono::high_resolution_clock::now();
        time = chrono::duration_cast<::chrono::duration<float>>(t_now - t_start).count();
        model = glm::translate(model, glm::vec3(-1.4f, 0.0f, 0.764f));

        model = glm::rotate(model, 0.5f * time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        modelLoc = glGetUniformLocation(program.GetProgram(), "model");
        GL_CHECK_ERRORS;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        GL_CHECK_ERRORS;
        upload_texture(program, sponge_texture);
        model5.Draw(program);

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
    glfwTerminate();
    return 0;
}
