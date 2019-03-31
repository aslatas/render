
#include "Camera.h"

#define CAMERA_ANGLE_LIMIT 0.0001f

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
    float half_pi = glm::half_pi<float>();
    cam->rotation.y += radians;
    if (cam->rotation.y >= half_pi - CAMERA_ANGLE_LIMIT) cam->rotation.y = half_pi - CAMERA_ANGLE_LIMIT;
    if (cam->rotation.y <= -half_pi + CAMERA_ANGLE_LIMIT) cam->rotation.y = -half_pi + CAMERA_ANGLE_LIMIT;
}

void Camera::ApplyInput(float delta, Controller *controller, Camera *cam, glm::vec3 axis_input)
{
    if (glm::length(axis_input) > 0.0f) {
        controller->acceleration = glm::vec3(0.0f);
        controller->acceleration += GetForwardVector(cam) * axis_input.x;
        controller->acceleration += GetRightVector(cam) * axis_input.y;
        controller->acceleration += glm::vec3(0.0f, 0.0f, axis_input.z);
        if (glm::length(controller->acceleration) > 1.0f) controller->acceleration = glm::normalize(controller->acceleration);
        controller->acceleration*= controller->max_acceleration;
    } else if (glm::length(controller->velocity) > 0.0f) {
        controller->acceleration = -glm::normalize(controller->velocity) * controller->brake_deceleration;
    } else {
        controller->acceleration = glm::vec3(0.0f);
        controller->velocity = glm::vec3(0.0f);
    }
    controller->velocity *= 1.0f - controller->friction * delta;
    controller->velocity += controller->acceleration * delta;
    float speed_multiplier = GetSpeedMultiplier();
    if (glm::length(controller->velocity) > controller->max_speed * speed_multiplier) controller->velocity = glm::normalize(controller->velocity) * controller->max_speed * speed_multiplier;
    cam->location += controller->velocity * delta;
}