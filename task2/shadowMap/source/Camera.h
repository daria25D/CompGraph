#pragma once

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <chrono>

#define WORLD_UP glm::vec3(0.0, 1.0, 0.0)

enum DIRECTION {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

class Camera {
    int width, height;
    const float CAMERA_SPEED_MULTIPLIER = 1.2f;
    glm::vec3 cameraPosition, cameraFront, cameraUp, cameraRight;
    float yaw = -90.0f, pitch = 0.0f;
    float fov = 45.0f;

    glm::mat4 view, proj;

    float deltaTime = 0.0f;
    std::chrono::high_resolution_clock::time_point lastFrame;

    void moveCameraPosition(const glm::vec3 &offset);
    void updateCameraVectors();
    void updateViewProjMatrices();

public:
    explicit Camera(const glm::vec3 &cam_pos, const glm::vec3 &cam_front, const glm::vec3 &cam_up, int w, int h);
    const glm::vec3 &getCameraPosition();
    const glm::vec3 &getCameraFront();
    const glm::vec3 &getCameraUp();

    float getCameraFov() const;
    float getCameraSpeed() const;

    glm::mat4 getViewMatrix();
    glm::mat4 getProjMatrix();

    void updateCurrentTime();

    void processCameraMovement(DIRECTION direction);
    void processCameraRotationOnMouse(float x_offset, float y_offset, bool constrain_pitch = true);
    void processCameraScroll(float y_offset);

    void setCameraPosition(const glm::vec3 &cam_pos);
    void setCameraFront(const glm::vec3 &cam_front);
    void setCameraUp(const glm::vec3 &cam_up);
    void setCameraFov(float new_fov);
};

Camera *get_camera();
void init_camera(const glm::vec3 &cam_pos, const glm::vec3 &cam_front, const glm::vec3 &cam_up, int w, int h);
void delete_camera();
