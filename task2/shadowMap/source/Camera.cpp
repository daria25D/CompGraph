#include <memory>
#include "Camera.h"

static std::unique_ptr<Camera> camera;

Camera *get_camera() {
    return camera.get();
}

void init_camera(const glm::vec3 &cam_pos, const glm::vec3 &cam_front, const glm::vec3 &cam_up) {
    camera = std::make_unique<Camera>(cam_pos, cam_front, cam_up);
}

void delete_camera() {
    camera.reset();
}

Camera::Camera(const glm::vec3 &cam_pos, const glm::vec3 &cam_front, const glm::vec3 &cam_up) :
    cameraPosition(cam_pos), cameraFront(cam_front), cameraUp(cam_up)
{}

const glm::vec3 &Camera::getCameraPosition() {
    return cameraPosition;
}

const glm::vec3 &Camera::getCameraFront() {
    return cameraFront;
}

const glm::vec3 &Camera::getCameraUp() {
    return cameraUp;
}

void Camera::setCameraPosition(const glm::vec3 &cam_pos) {
    cameraPosition = cam_pos;
}

void Camera::setCameraFront(const glm::vec3 &cam_front) {
    cameraFront = cam_front;
}

void Camera::setCameraUp(const glm::vec3 &cam_up) {
    cameraUp = cam_up;
}
