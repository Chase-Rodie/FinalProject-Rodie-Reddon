#ifndef CAMERA_H
#define CAMERA_H

#include "graphics_headers.h"

class Camera
{
public:
    Camera();
    ~Camera();
    bool Initialize(int w, int h);
    glm::mat4 GetProjection();
    glm::mat4 GetView();
    void ProcessKeyboard(std::string direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
    void ProcessMouseScroll(float yoffset);
    void UpdateView();
    void FaceDirection(const glm::vec3& target);
    void SetLookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up);



    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    void SetPosition(const glm::vec3& pos);
    void SetTarget(const glm::vec3& target);
    void SetUp(const glm::vec3& upVec);




private:

    glm::vec3 cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    float yaw = -90.0f;   // Horizontal look direction
    float pitch = 0.0f;   // Vertical look direction
    float fov = 40.0f;    // Field of View


    double x = 0.0;
    double y = 10.0;
    double z = -16.0;
    glm::mat4 projection;
    glm::mat4 view;

    void updateCameraVectors(); // helper

};

#endif /* CAMERA_H */
