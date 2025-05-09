#include "camera.h"

Camera::Camera()
{

}

Camera::~Camera()
{

}

bool Camera::Initialize(int w, int h)
{
    updateCameraVectors(); // setup initial direction
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    projection = glm::perspective(glm::radians(fov), float(w) / float(h), 0.01f, 100.0f);
    return true;
}

glm::mat4 Camera::GetProjection()
{
    return projection;
}

glm::mat4 Camera::GetView()
{
    return view;
}

void Camera::ProcessKeyboard(std::string direction, float deltaTime)
{
    float velocity = deltaTime * 5.0f;

    if (direction == "FORWARD")
        cameraPos += cameraFront * velocity;
    if (direction == "BACKWARD")
        cameraPos -= cameraFront * velocity;
    if (direction == "LEFT")
        cameraPos -= cameraRight * velocity;
    if (direction == "RIGHT")
        cameraPos += cameraRight * velocity;

    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (constrainPitch)
    {
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    fov -= yoffset;
    if (fov < 10.0f) fov = 10.0f;
    if (fov > 90.0f) fov = 90.0f;

    projection = glm::perspective(glm::radians(fov), 4.0f / 3.0f, 0.01f, 100.0f);
}

void Camera::updateCameraVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);

    cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));

    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

void Camera::UpdateView()
{
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

void Camera::SetPosition(const glm::vec3& pos) {
    cameraPos = pos;
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

void Camera::SetTarget(const glm::vec3& target) {
    cameraFront = glm::normalize(target - cameraPos);
    cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
    UpdateView();
}

void Camera::SetUp(const glm::vec3& upVec) {
    worldUp = upVec;
    cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
    UpdateView();
}



void Camera::FaceDirection(const glm::vec3& target)
{
    glm::vec3 direction = glm::normalize(target - cameraPos);

    pitch = glm::degrees(asin(direction.y));
    yaw = glm::degrees(atan2(direction.z, direction.x));

    updateCameraVectors();
}
