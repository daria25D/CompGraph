#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
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

vector<Object> setup_objects_from_file(const string &filename) {
    vector<Object> allObjects;
    //TODO add reading from json file
    //TODO check if it is a json file
    return allObjects;
}