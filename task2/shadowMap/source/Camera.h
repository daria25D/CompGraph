#pragma once

#include <glm/glm.hpp>
#include <chrono>

class Camera {

    glm::vec3 cameraPosition, cameraFront, cameraUp;
    const float CAMERA_SPEED_MULTIPLIER = 1.2f;
    float deltaTime = 0.0f;
    std::chrono::high_resolution_clock::time_point lastFrame;

public:
    explicit Camera(const glm::vec3 &cam_pos, const glm::vec3 &cam_front, const glm::vec3 &cam_up);
    const glm::vec3 &getCameraPosition();
    const glm::vec3 &getCameraFront();
    const glm::vec3 &getCameraUp();

    float getCameraSpeed() const;

    void moveCameraPosition(const glm::vec3 &offset);
    void updateCurrentTime();

    void setCameraPosition(const glm::vec3 &cam_pos);
    void setCameraFront(const glm::vec3 &cam_front);
    void setCameraUp(const glm::vec3 &cam_up);
};

Camera *get_camera();
void init_camera(const glm::vec3 &cam_pos, const glm::vec3 &cam_front, const glm::vec3 &cam_up);
void delete_camera();
