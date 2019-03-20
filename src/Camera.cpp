
#include "Camera.h"
#include <iostream>

glm::mat4 Camera::GetViewTransform(Camera *cam)
{
    glm::mat4 rot = glm::mat4(1.0f);
    glm::mat4 trans = glm::mat4(1.0f);
    
    rot = glm::rotate(rot, cam->rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    rot = glm::rotate(rot, cam->rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    rot = glm::rotate(rot, cam->rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    trans = glm::translate(trans, cam->location);
    
    return glm::lookAt(cam->location, cam->location + GetForwardVector(cam), glm::vec3(0.0f, 0.0f, 1.0f));
    return (cam->mode == FPS) ? rot * trans : trans * rot;
    //return glm::translate(glm::mat4_cast(cam->rotation), -cam->location);
    //return glm::lookAt(cam->location, cam->location + GetForwardVector(cam), GetUpVector(cam));
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