#include <memory>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"

static std::unique_ptr<Camera> camera;

Camera *get_camera() {
    return camera.get();
}

void init_camera(const glm::vec3 &cam_pos, const glm::vec3 &cam_front, const glm::vec3 &cam_up, int w, int h) {
    camera = std::make_unique<Camera>(cam_pos, cam_front, cam_up, w, h);
}

void delete_camera() {
    camera.reset();
}

Camera::Camera(const glm::vec3 &cam_pos, const glm::vec3 &cam_front, const glm::vec3 &cam_up, int w, int h) :
    cameraPosition(cam_pos), cameraFront(cam_front), cameraUp(cam_up), width(w), height(h)
{
    lastFrame = std::chrono::high_resolution_clock::now();
    updateCameraVectors();
    updateViewProjMatrices();
}

const glm::vec3 &Camera::getCameraPosition() {
    return cameraPosition;
}

const glm::vec3 &Camera::getCameraFront() {
    return cameraFront;
}

const glm::vec3 &Camera::getCameraUp() {
    return cameraUp;
}

float Camera::getCameraFov() const {
    return fov;
}

float Camera::getCameraSpeed() const {
    return deltaTime * CAMERA_SPEED_MULTIPLIER;
}

glm::mat4 Camera::getViewMatrix() {
    return view;
}

glm::mat4 Camera::getProjMatrix() {
    return proj;
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

void Camera::setCameraFov(const float new_fov) {
    fov = new_fov;
}

void Camera::moveCameraPosition(const glm::vec3 &offset) {
    cameraPosition += offset;
}

void Camera::updateCurrentTime() {
    auto currentFrame = std::chrono::high_resolution_clock::now();
    deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentFrame - lastFrame).count();
    lastFrame = currentFrame;
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    cameraFront = glm::normalize(front);
    cameraRight = glm::normalize(glm::cross(cameraFront, WORLD_UP));
    cameraUp    = glm::normalize(glm::cross(cameraRight, cameraFront));
}

void Camera::updateViewProjMatrices() {
    proj = glm::perspective(glm::radians(fov), (float) width / (float) height, 0.1f, 100.0f);
    view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
}

void Camera::processCameraMovement(DIRECTION direction) {
    if (direction == FORWARD)
        moveCameraPosition(getCameraSpeed() * getCameraFront());
    else if (direction == BACKWARD)
        moveCameraPosition(-getCameraSpeed() * getCameraFront());
    else if (direction == LEFT)
        moveCameraPosition(-getCameraSpeed() *
                glm::normalize(glm::cross(getCameraFront(), getCameraUp())));
    else if (direction == RIGHT)
        moveCameraPosition(getCameraSpeed() *
                           glm::normalize(glm::cross(getCameraFront(), getCameraUp())));

    updateViewProjMatrices();
}

void Camera::processCameraRotationOnMouse(float x_offset, float y_offset, bool constrain_pitch) {
    yaw   += x_offset;
    pitch += y_offset;
    if (constrain_pitch) {
        if (pitch > 89.0f)
            pitch  = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    updateCameraVectors();
    updateViewProjMatrices();
}

void Camera::processCameraScroll(float y_offset) {
    fov -= y_offset;
    if (fov < 1.0f)
        fov = 1.0f;
    else if (fov > 45.0f)
        fov = 45.0f;
    updateViewProjMatrices();
}
