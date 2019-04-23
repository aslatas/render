#include "Bounds.h"

AABB_3D* Create3DAxisAlignedBoundingBox(float* min, float* max)
{
    AABB_3D *aabb = (AABB_3D *)malloc(sizeof(AABB_3D));
    aabb->min = glm::make_vec3(&min[0]);
    aabb->max = glm::make_vec3(&max[0]);
    aabb->ext = (aabb->max - aabb->min) / 2.0f;
    aabb->center = aabb->min + aabb->ext;

    // printf("Extent: (%f, %f, %f)\n", aabb->ext[0], aabb->ext[1], aabb->ext[2]);
    // printf("Center: (%f, %f, %f)\n", aabb->center[0], aabb->center[1], aabb->center[2]);

    return aabb;
}

bool CheckAxisPointOverlapp(float min_x, float max_x, float min_y, float max_y)
{
    return (min_x <= min_y && min_y <= max_x && max_x >= max_y && max_y >= min_x) || // X--y------y--X
        (min_x >= min_y && min_y <= max_x && max_x >= max_y && max_y >= min_x) || // y--X------y--X
        (min_x <= min_y && min_y <= max_x && max_x <= max_y && max_y >= min_x) || // X--y------X--y
        (min_x >= min_y && min_y <= max_x && max_x <= max_y && max_y >= min_x);   // y--X------X--y
}

// TODO(Matt): Is a slab test faster than point-axis tests?
bool CheckBoundingBoxCollision2D(AABB_2D &boxA, AABB_2D &boxB)
{
    // return CheckAxisPointOverlapp(boxA.min[0], boxA.max[0], boxB.min[0], boxB.max[0]) &&
    //     CheckAxisPointOverlapp(boxA.min[1], boxA.max[1], boxB.min[1], boxB.max[1]);
    return false;
}

// TODO(Matt): Is a slab test faster than point-axis tests?
bool CheckBoundingBoxCollision3D(AABB_3D &boxA, AABB_3D &boxB)
{
    // return CheckAxisPointOverlapp(boxA.min[0], boxA.max[0], boxB.min[0], boxB.max[0]) &&
    //     CheckAxisPointOverlapp(boxA.min[1], boxA.max[1], boxB.min[1], boxB.max[1]) &&
    //     CheckAxisPointOverlapp(boxA.min[2], boxA.max[2], boxB.min[2], boxB.max[2]);

    return (abs(boxA.center[2] - boxB.center[2]) <= (boxA.ext[2] + boxB.ext[2])) && 
           (abs(boxA.center[0] - boxB.center[0]) <= (boxA.ext[0] + boxB.ext[0])) && 
           (abs(boxA.center[1] - boxB.center[1]) <= (boxA.ext[1] + boxB.ext[1]));
}
