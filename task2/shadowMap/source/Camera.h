#pragma once

#include <glm/glm.hpp>

class Camera {
    glm::vec3 cameraPosition, cameraFront, cameraUp;
public:
    explicit Camera(const glm::vec3 &cam_pos, const glm::vec3 &cam_front, const glm::vec3 &cam_up);
    const glm::vec3 &getCameraPosition();
    const glm::vec3 &getCameraFront();
    const glm::vec3 &getCameraUp();

    void setCameraPosition(const glm::vec3 &cam_pos);
    void setCameraFront(const glm::vec3 &cam_front);
    void setCameraUp(const glm::vec3 &cam_up);
};

Camera *get_camera();
void init_camera(const glm::vec3 &cam_pos, const glm::vec3 &cam_front, const glm::vec3 &cam_up);
void delete_camera();
