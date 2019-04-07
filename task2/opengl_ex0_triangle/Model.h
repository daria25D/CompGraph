#pragma once
#ifndef MODEL_H
#define MODEL_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL/SOIL.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "ShaderProgram.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

bool z_test = false;
GLuint FramebufferName = 0;
unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);

class Model
{
public:
    /*  Model Data */
    vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<Mesh> meshes;
    string directory;
    bool gammaCorrection;
    string type;

    /*  Functions   */
    // constructor, expects a filepath to a 3D model.
    Model(string const &path, string Type = "sphere", bool gamma = false) : gammaCorrection(gamma), type(Type)
    {
        loadModel(path);
    }

    // draws the model, and thus all its meshes
    void Draw(ShaderProgram shader)
    {
        for(unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

private:
    /*  Functions   */
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const &path)
    {
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // check for errors
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode *node, const aiScene *scene)
    {
        // process each mesh located at the current node
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene.
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }

    Mesh processMesh(aiMesh *mesh, const aiScene *scene)
    {
        // data to fill
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        // Walk through each of the mesh's vertices
        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // normals
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
            // texture coordinates
            if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            // tangent
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;
            // bitangent
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;
            vertices.push_back(vertex);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for(unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN


        if (type == "fabric") {
            Texture texture;
            texture.id = TextureFromFile("fabric_diffuse.dds", "../textures");
            texture.type = "texture_diffuse";
            texture.path = "fabric_diffuse.dds";
            textures.push_back(texture);
            textures_loaded.push_back(texture);
            texture.id = TextureFromFile("fabric_normal.dds", "../textures");
            texture.type = "texture_normal";
            texture.path = "fabric_normal.dds";
            textures.push_back(texture);
            textures_loaded.push_back(texture);
            texture.id = TextureFromFile("fabric_height.dds", "../textures");
            texture.type = "texture_height";
            texture.path = "fabric_height.dds";
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        } else if (type == "cup") {
            Texture texture;
            texture.id = TextureFromFile("quartz_diffuse.dds", "../textures");
            texture.type = "texture_diffuse";
            texture.path = "quartz_diffuse.dds";
            textures.push_back(texture);
            textures_loaded.push_back(texture);
            texture.id = TextureFromFile("quartz_normal.dds", "../textures");
            texture.type = "texture_normal";
            texture.path = "quartz_normal.dds";
            textures.push_back(texture);
            textures_loaded.push_back(texture);
            texture.id = TextureFromFile("quartz_height.dds", "../textures");
            texture.type = "texture_height";
            texture.path = "quartz_height.dds";
            textures.push_back(texture);
            textures_loaded.push_back(texture);

        } else if (type == "bowl") {
            Texture texture;
            texture.id = TextureFromFile("quartz_diffuse.dds", "../textures");
            texture.type = "texture_diffuse";
            texture.path = "quartz_diffuse.dds";
            textures.push_back(texture);
            textures_loaded.push_back(texture);
            texture.id = TextureFromFile("quartz_normal.dds", "../textures");
            texture.type = "texture_normal";
            texture.path = "quartz_normal.dds";
            textures.push_back(texture);
            textures_loaded.push_back(texture);
            texture.id = TextureFromFile("quartz_height.dds", "../textures");
            texture.type = "texture_height";
            texture.path = "quartz_height.dds";
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        } else if (type == "plane") {
            Texture texture;
            texture.id = TextureFromFile("wood_diffuse.dds", "../textures");
            texture.type = "texture_diffuse";
            texture.path = "wood_diffuse.dds";
            textures.push_back(texture);
            textures_loaded.push_back(texture);
            texture.id = TextureFromFile("wood_normal.dds", "../textures");
            texture.type = "texture_normal";
            texture.path = "wood_normal.dds";
            textures.push_back(texture);
            textures_loaded.push_back(texture);
            texture.id = TextureFromFile("wood_height.dds", "../textures");
            texture.type = "texture_height";
            texture.path = "wood_height.dds";
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }

        // return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices, textures);
    }
};


unsigned int TextureFromFile(const char *path, const string &directory, bool gamma)
{
    string filename = string(path);
    filename = directory + '/' + filename;
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = SOIL_load_image(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        SOIL_free_image_data(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        SOIL_free_image_data(data);
    }

    return textureID;
}
#endif