#include "Bounds.h"

AABB_2D* Create2DAxisAlignedBoundingBox(float* min, float* max)
{
    AABB_2D *aabb = (AABB_2D *)malloc(sizeof(AABB_2D));
    aabb->min = glm::make_vec2(&min[0]);
    aabb->max = glm::make_vec2(&max[0]);
    aabb->ext = (aabb->max - aabb->min) / 2.0f;
    aabb->center = aabb->min + aabb->ext;

    return aabb;
}

AABB_3D* Create3DAxisAlignedBoundingBox(float* min, float* max)
{
    AABB_3D *aabb = (AABB_3D *)malloc(sizeof(AABB_3D));
    aabb->min = glm::make_vec3(&min[0]);
    aabb->max = glm::make_vec3(&max[0]);
    aabb->ext = (aabb->max - aabb->min) / 2.0f;
    aabb->center = aabb->min + aabb->ext;

    return aabb;
}

// SIMD approach to AABB-AABB overlapp
bool CheckBoundingBoxCollision2D(AABB_2D &boxA, AABB_2D &boxB)
{
    return (abs(boxA.center[1] - boxB.center[1]) <= (boxA.ext[1] + boxB.ext[1])) && 
           (abs(boxA.center[0] - boxB.center[0]) <= (boxA.ext[0] + boxB.ext[0]));
}

// SIMD approach to AABB-AABB overlapp
bool CheckBoundingBoxCollision3D(AABB_3D &boxA, AABB_3D &boxB)
{
    return (abs(boxA.center[2] - boxB.center[2]) <= (boxA.ext[2] + boxB.ext[2])) && 
           (abs(boxA.center[0] - boxB.center[0]) <= (boxA.ext[0] + boxB.ext[0])) && 
           (abs(boxA.center[1] - boxB.center[1]) <= (boxA.ext[1] + boxB.ext[1]));
}
