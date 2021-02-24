#pragma once
#ifndef MESH_H
#define MESH_H

#include <glm/glm.hpp>
#include <vector>

using namespace std;

class ShaderProgram;

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

struct Texture {
    unsigned int id;
    string type;
    string path;
};

class Mesh {
public:
    /*  Mesh Data  */
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    unsigned int VAO;

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);

    void Draw(const ShaderProgram &shader);

private:
    /*  Render data  */
    unsigned int VBO, EBO;

    // initializes all the buffer objects/arrays
    void setupMesh();
};

#endif