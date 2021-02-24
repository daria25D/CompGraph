#pragma once
#ifndef MODEL_H
#define MODEL_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL/SOIL.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>

#include "Mesh.h"

using namespace std;

class ShaderProgram;

unsigned int TextureFromFile(const char *path, const string &directory);

class Model {
public:
    /*  Model Data */
    vector<Texture> textures_loaded; // stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<Mesh> meshes;
    string directory;
    bool gammaCorrection;
    string type;

    // expects a filepath to a 3D model.
    Model(string const &path, string Type, bool gamma = false);

    // draws the model, and thus all its meshes
    void Draw(const ShaderProgram &shader);

private:
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const &path);

    vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                         const string &typeName);

    // processes a node in a recursive fashion.
    // Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode *node, const aiScene *scene);

    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
};

#endif