#ifndef BOUNDS_H

// Bounding region for 2D space
//   min: minimum x,y values located in the model
//   max: maximum x,y values located in the model
struct AABB_2D
{
    glm::vec2 min;
    glm::vec2 max;
    glm::vec2 center; // center of the box
    glm::vec2 ext; // half length
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

enum HalfSpace
{
    NEGATIVE = 0x00, // outside
    ON_PLANE = 0x01,  // on the plane
    POSITIVE = 0x02   // inside
};

AABB_2D* Create2DAxisAlignedBoundingBox(float* min, float* max);
AABB_3D* Create3DAxisAlignedBoundingBox(float* min, float* max);
AABB_3D* Create3DAxisAlignedBoundingBoxFromCenter(float* center, float* ext);

// TODO(Matt): Is a slab test faster than point-axis tests?
bool CheckBoundingBoxCollision2D(AABB_2D &boxA, AABB_2D &boxB);

// TODO(Matt): Is a slab test faster than point-axis tests?
bool CheckBoundingBoxCollision3D(AABB_3D &boxA, AABB_3D &boxB);


// Determine if a point is in Positive/Negative Space or Intersecting
// the plane.
HalfSpace ClassifyPoint(glm::vec4 plane, glm::vec3 point);

// Calculate a AABB-Frustum intersection
bool AABBFrustumIntersection(Camera::Frustum *frustum, AABB_3D *aabb);

#define BOUNDS_H
#endif