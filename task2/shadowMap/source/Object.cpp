#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <iostream>
#include "Object.h"

Object::Object(const string &name, const string &path_to_obj_file, const glm::mat4 &transform) :
    model(path_to_obj_file, name), transform(transform) {}

void Object::Draw(const ShaderProgram &shader) {
    model.Draw(shader);
}

const glm::mat4 &Object::getTransform() {
    return transform;
}

glm::vec3 Object::getPosition() {
    return glm::vec3(transform[3]);
}

void Object::setTransform(const glm::mat4 &new_transform) {
    transform = new_transform;
}

void Object::rotateObject(float rotation_angle, const glm::vec3 &rotation_axis) {
    transform = glm::rotate(transform, glm::radians(rotation_angle), rotation_axis);
}

void Object::scaleObject(float scale_multiplier) {
    transform = glm::scale(transform, glm::vec3(scale_multiplier));
}

void Object::scaleObject(const glm::vec3 &scale_multiplier) {
    transform = glm::scale(transform, scale_multiplier);
}

void Object::translateObject(const glm::vec3 &translate_vector) {
    transform = glm::translate(transform, translate_vector);
}

static Object setup_one_object_from_json(const rapidjson::Value& doc) {
    string name = doc["name"].GetString();
    string path = doc["path"].GetString();
    glm::mat4 transform = glm::mat4(0.0);
    int i = 0, j = 0;
    for (const auto &num : doc["transform"].GetArray()) {
        transform[i][j] = num.GetFloat();
        i = (i + 1) % 3;
        if (i == 0)
            j++;
    }

    return Object(name, path, transform);
}

vector<Object> setup_objects_from_json(const string &filename) {
    std::ifstream ifs(filename);
    rapidjson::IStreamWrapper isw(ifs);

    vector<Object> allObjects;

    rapidjson::Document doc;
    doc.ParseStream(isw);
    assert(doc.HasMember("objects"));
    for (const auto &object : doc["objects"].GetArray()) {
        allObjects.push_back(setup_one_object_from_json(object));
    }
    return allObjects;
}