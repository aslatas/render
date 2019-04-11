
#ifndef CAMERA_H

namespace Camera
{
    enum CameraMode
    {
        FPS, TARGET
    };
    
    struct Camera
    {
        float fov = 45.0f;
        float aspect_ratio = 16.0f / 9.0f;
        float near_dist = 0.01f;
        float far_dist = 1000.0f;
        glm::vec3 location = glm::vec3(0.0f);
        glm::vec3 rotation = glm::vec3(0.0f);
        CameraMode mode = FPS;
    };
    glm::mat4 GetViewTransform(Camera *cam);
    glm::mat4 GetProjectionTransform(const Camera *cam);
    glm::vec3 GetRotation(const Camera *cam);
    glm::vec3 GetForwardVector(const Camera *cam);
    glm::vec3 GetRightVector(const Camera *cam);
    void MoveForward(Camera *cam, float distance);
    void MoveRight(Camera *cam, float distance);
    void MoveUp(Camera *cam, float distance);
    void AddYaw(Camera *cam, float radians);
    void AddPitch(Camera *cam, float radians);
    
    struct Controller
    {
        float max_acceleration;
        float brake_deceleration;
        float max_speed;
        float friction;
        glm::vec3 velocity;
        glm::vec3 acceleration;
    };
    
    void ApplyInput(float delta, Controller *controller, Camera *cam, glm::vec3 axis_input);
}

#define CAMERA_H
#endif