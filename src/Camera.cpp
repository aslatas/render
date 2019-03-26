
#include "Camera.h"

#define CAMERA_ANGLE_LIMIT 0.01f

glm::mat4 Camera::GetViewTransform(Camera *cam)
{
    return glm::lookAt(cam->location, cam->location + GetForwardVector(cam), glm::vec3(0.0f, 0.0f, 1.0f));
}

glm::mat4 Camera::GetProjectionTransform(const Camera *cam)
{
    glm::mat4 transform = glm::perspective(cam->fov, cam->aspect_ratio, cam->near_dist, cam->far_dist);
    transform[1][1] *= -1;
    return transform;
}

glm::vec3 Camera::GetForwardVector(const Camera *cam)
{
    glm::vec3 forward;
    forward.x = cos(cam->rotation.z) * cos(cam->rotation.y);
    forward.y = sin(cam->rotation.z) * cos(cam->rotation.y);
    forward.z = sin(cam->rotation.y);
    return glm::normalize(forward);
}

glm::vec3 Camera::GetRightVector(const Camera *cam)
{
    return glm::cross(GetForwardVector(cam), glm::vec3(0.0f, 0.0f, 1.0f));
}

void Camera::MoveForward(Camera *cam, float distance)
{
    cam->location += GetForwardVector(cam) * distance;
}

void Camera::MoveRight(Camera *cam, float distance)
{
    cam->location += GetRightVector(cam) * distance;
}


void Camera::MoveUp(Camera *cam, float distance)
{
    cam->location += glm::vec3(0.0f, 0.0f, 1.0f) * distance;
}

void Camera::AddYaw(Camera *cam, float radians)
{
    cam->rotation.z += radians;
}

void Camera::AddPitch(Camera *cam, float radians)
{
    cam->rotation.y += radians;
    if (cam->rotation.y >= glm::half_pi<float>()) cam->rotation.y = glm::half_pi<float>() - CAMERA_ANGLE_LIMIT;
    if (cam->rotation.y <= -glm::half_pi<float>()) cam->rotation.y = -glm::half_pi<float>() + CAMERA_ANGLE_LIMIT;
}