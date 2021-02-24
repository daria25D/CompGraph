#ifndef MAIN_OBJECT_H
#define MAIN_OBJECT_H

#include <vector>
#include <glm/mat4x4.hpp> // glm::mat4
#include "rapidjson/document.h"

#include "Model.h"

class ShaderProgram;

class Object {
    glm::mat4 transform;
    Model model;
public:
    explicit Object(const string &name, const string &path_to_obj_file, const glm::mat4 &transform);

    const glm::mat4 &getTransform();
    glm::vec3 getPosition();

    void setTransform(const glm::mat4 &new_transform);

    //TODO add animation data
    //TODO add ability to set rotation_axis as char (x, y, z)
    //pass angle in degrees
    void rotateObject(float rotation_angle, const glm::vec3 &rotation_axis);
    void scaleObject(float scale_multiplier);
    void scaleObject(const glm::vec3 &scale_multiplier);
    void translateObject(const glm::vec3 &translate_vector);

    void Draw(const ShaderProgram &shader, const glm::mat4 &proj, const glm::mat4 &view);
    void DrawToDepth(ShaderProgram &shader);
};

vector<Object> setup_objects_from_json(const string &filename);

#endif //MAIN_OBJECT_H
