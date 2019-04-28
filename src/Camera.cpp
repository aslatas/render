
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

Camera::Frustum *Camera::ExtractFrustumPlanes(Camera &cam, glm::mat4 *modelview)
{
    glm::mat4 proj = GetProjectionTransform(&cam);
    // glm::mat4 proj = glm::perspective(cam.fov, cam.aspect_ratio, cam.near_dist, cam.far_dist);

    Frustum* f = (Frustum *)malloc(sizeof(Frustum)); 
    // glm::vec4 *planes = (glm::vec4*)malloc(6 * sizeof(glm::vec4));

    glm::mat4 frustum = (modelview == nullptr) ? proj : proj * *modelview;
    frustum = inverse(frustum);

    // left
    f->planes[0][0] = 1*(frustum[3][0] + frustum[0][0]);
    f->planes[0][1] = 1*(frustum[3][1] + frustum[0][1]);
    f->planes[0][2] = 1*(frustum[3][2] + frustum[0][2]);
    f->planes[0][3] = 1*(frustum[3][3] + frustum[0][3]);

    // right
    f->planes[1][0] = 1*(frustum[3][0] - frustum[0][0]);
    f->planes[1][1] = 1*(frustum[3][1] - frustum[0][1]);
    f->planes[1][2] = 1*(frustum[3][2] - frustum[0][2]);
    f->planes[1][3] = 1*(frustum[3][3] - frustum[0][3]);

    // bottom
    f->planes[2][0] = 1*(frustum[3][0] + frustum[1][0]);
    f->planes[2][1] = 1*(frustum[3][1] + frustum[1][1]);
    f->planes[2][2] = 1*(frustum[3][2] + frustum[1][2]);
    f->planes[2][3] = 1*(frustum[3][3] + frustum[1][3]);
    
    // top
    f->planes[3][0] = 1*(frustum[3][0] - frustum[1][0]);
    f->planes[3][1] = 1*(frustum[3][1] - frustum[1][1]);
    f->planes[3][2] = 1*(frustum[3][2] - frustum[1][2]);
    f->planes[3][3] = 1*(frustum[3][3] - frustum[1][3]);

    // near
    f->planes[4][0] = 1*(frustum[3][0] + frustum[2][0]);
    f->planes[4][1] = 1*(frustum[3][1] + frustum[2][1]);
    f->planes[4][2] = 1*(frustum[3][2] + frustum[2][2]);
    f->planes[4][3] = 1*(frustum[3][3] + frustum[2][3]);
    
    // far
    f->planes[5][0] = 1*(frustum[3][0] - frustum[2][0]);
    f->planes[5][1] = 1*(frustum[3][1] - frustum[2][1]);
    f->planes[5][2] = 1*(frustum[3][2] - frustum[2][2]);
    f->planes[5][3] = 1*(frustum[3][3] - frustum[2][3]);

    // normalize
    // TODO(Dustin): use GLM math
    for (int i = 0; i < 6; ++i)
    {
        float mag = 1 / sqrt(f->planes[i][0] + 
                         f->planes[i][1] + 
                         f->planes[i][2] + 
                         f->planes[i][3]);
        f->planes[i][0] *= mag;
        f->planes[i][1] *= mag;
        f->planes[i][2] *= mag;
        f->planes[i][3] *= mag;
    }

    // Now get the four corners of the near and far plane
/*
Near Top Left = Cnear + (up * (Hnear / 2)) - (w * (Wnear / 2))

Near Top Right = Cnear + (up * (Hnear / 2)) + (w * (Wnear / 2))

Near Bottom Left = Cnear - (up * (Hnear / 2)) - (w * (Wnear /2))

Near Bottom Right = Cnear - (up * (Hnear / 2)) + (w * (Wnear / 2))

Far Top Left = Cfar + (up * (Hfar / 2)) - (w * Wfar / 2))

Far Top Right = Cfar + (up * (Hfar / 2)) + (w * Wfar / 2))

Far Bottom Left = Cfar - (up * (Hfar / 2)) - (w * Wfar / 2))

Far Bottom Right = Cfar - (up * (Hfar / 2)) + (w * Wfar / 2))
*/
    // different vectors from the camera
    glm::vec3 forward = GetForwardVector(&cam);
    glm::vec3 w   = GetRightVector(&cam);
    glm::vec3 up      = glm::vec3(0.0f, 0.0f, 1.0f);

    // near poin
    float hnear = 2.0f * tan(cam.fov / 2.0f) * cam.near_dist;
    float wnear = hnear * cam.aspect_ratio;

    float hfar = 2 * tan(cam.fov / 2) * cam.far_dist;
    float wfar = hfar * cam.aspect_ratio;

    // centers for each plane
    glm::vec3 cnear = cam.location + forward * cam.near_dist;
    glm::vec3 cfar = cam.location + forward * cam.far_dist;

    // let's get the point
    f->points[0] = cnear + (up * (hnear / 2.0f)) - (w * (wnear / 2.0f));
    f->points[1] = cnear + (up * (hnear / 2.0f)) + (w * (wnear / 2.0f));
    f->points[2] = cnear - (up * (hnear / 2.0f)) - (w * (wnear /2.0f));
    f->points[3] = cnear - (up * (hnear / 2.0f)) + (w * (wnear / 2.0f));
    f->points[4] = cfar + (up * (hfar / 2.0f)) - (w * wfar / 2.0f);
    f->points[5] = cfar + (up * (hfar / 2.0f)) + (w * wfar / 2.0f);
    f->points[6] = cfar - (up * (hfar / 2.0f)) - (w * wfar / 2.0f);
    f->points[7] = cfar - (up * (hfar / 2.0f)) + (w * wfar / 2.0f);

    return f;
}