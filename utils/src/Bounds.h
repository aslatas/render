#ifndef BOUNDS_H

// Bounding region for 2D space
//   min: minimum x,y values located in the model
//   max: maximum x,y values located in the model
struct AABB_2D
{
    glm::vec3 min[2];
    glm::vec3 max[2];
};
// Bounding region for 3D space
//   min: minimum x,y,z values located in the model
//   max: maximum x,y,z values located in the model
struct AABB_3D
{
    glm::vec3 min;
    glm::vec3 max;
    glm::vec3 center; // center of the box
    glm::vec3 ext; // half length
};

AABB_3D* Create3DAxisAlignedBoundingBox(float* min, float* max);

// TODO(Matt): Rework this to use fmin/fmax instead of branching, for 
// M A X I M U M    S P E E D.
bool CheckAxisPointOverlapp(float min_x, float max_x, float min_y, float max_y);

// TODO(Matt): Is a slab test faster than point-axis tests?
bool CheckBoundingBoxCollision2D(AABB_2D &boxA, AABB_2D &boxB);

// TODO(Matt): Is a slab test faster than point-axis tests?
bool CheckBoundingBoxCollision3D(AABB_3D &boxA, AABB_3D &boxB);

#define BOUNDS_H
#endif