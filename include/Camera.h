
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

    /*
     * planes: a six element vec4 array containing each of the size planes. Each vec4
     *         are the normals to the plane (x, y, z, w).
     *   index | plane
     *   ------|-------
     *     0   | left
     *     1   | right
     *     2   | bottom
     *     3   | top
     *     4   | near
     *     5   | far
     *         Planes are returned normalized.
     * points: 8 points represenenting 4 corners of the near plane and 4 corners of the far plane.
     *   index | point
     *   ------|-------
     *     0   | Near Top Left
     *     1   | Near Top Right
     *     2   | Near Bottom Left
     *     3   | Near Bottom Right
     *     4   | Far Top Left
     *     5   | Far Top Right
     *     6   | Far Bottom Left
     *     7   | Far Bottom Right
     */
    struct Frustum
    {
        glm::vec4 planes[6];
        glm::vec3 points[8];
    };

    /*
     * Extracts frustum planes from a camera.
     * cam: camera to extract the frustum planes from
     * modelview: optional parameter representing the model-view matrix.
     * 1. If the modelview is not provided, then only the projection matrix is used. 
     *    The algorithm gives the clipping planesin view space (i.e., camera space).
     * 2. If the  modelview  equal  to  the modelview matrices, then  the  algorithm  
     *    gives  the clipping  planes in model space
     * Planes are returned normalized.
     */
    Frustum *ExtractFrustumPlanes(Camera &cam, glm::mat4 *modelview = nullptr);

    /*
     * Extracts frustum planes from a camera.
     * cam: camera to extract the frustum planes from
     * modelview: optional parameter representing the model-view matrix.
     * 1. If the modelview is not provided, then only the projection matrix is used. 
     *    The algorithm gives the clipping planesin view space (i.e., camera space).
     * 2. If the  modelview  equal  to  the modelview matrices, then  the  algorithm  
     *    gives  the clipping  planes in model space
     * Planes are NOT returned normalized.
     */
    Frustum *UExtractFrustumPlanes(Camera &cam, glm::mat4 *modelview = nullptr);
}

#define CAMERA_H
#endif