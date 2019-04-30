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

AABB_3D* Create3DAxisAlignedBoundingBoxFromCenter(float* center, float* ext)
{
    AABB_3D *aabb = (AABB_3D *)malloc(sizeof(AABB_3D));
    aabb->ext = glm::make_vec3(&ext[0]);
    aabb->center = glm::make_vec3(&center[0]);
    aabb->min = aabb->center - aabb->ext;
    aabb->max = aabb->center + aabb->ext; 

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

// Determine if a point is in Positive/Negative Space or Intersecting
// the plane.
HalfSpace ClassifyPoint(glm::vec4 plane, glm::vec3 point)
{
    float d = plane[0] * point[0] +
              plane[1] * point[1] +
              plane[2] * point[2] +
              plane[3];

    if (d < 0) return NEGATIVE;
    if (d > 0) return POSITIVE;
    return ON_PLANE;
}

bool AABBFrustumIntersection(Camera::Frustum *frustum, AABB_3D *aabb)
{
    glm::vec4 *frustum_planes = frustum->planes;
    glm::vec3 *frustum_points = frustum->points;
    
    // unsigned char intersection_min = 0x03; // assume inside
    // unsigned char intersection_max = 0x03; // assume inside
    // for (int i = 0; i < 6; ++i)
    // {
    //     // determine if the point is inside, on, or outside the plane
    //     // looking for a single case of outside. All points for a min/max
    //     // should exist in Positive Space or On the plane to be inside the
    //     // Frustum. Once intersection is locked at 0x00, it stays there.
    //     // HalfSpace res = ClassifyPoint(frustum_planes[i], aabb->min);
    //     intersection_min &= ClassifyPoint(frustum_planes[i], aabb->min);
    //     intersection_max &= ClassifyPoint(frustum_planes[i], aabb->max);
    // }
    

    glm::vec3 min = aabb->min;
    glm::vec3 max = aabb->max;
    for (int i = 0; i < 6; ++i)
    {
        int out = 0;
        out += ((dot( frustum_planes[i], glm::vec4(min[0], min[1], min[2], 1.0f) ) < 0.0 )?1:0);
        out += ((dot( frustum_planes[i], glm::vec4(max[0], min[1], min[2], 1.0f) ) < 0.0 )?1:0);
        out += ((dot( frustum_planes[i], glm::vec4(min[0], max[1], min[2], 1.0f) ) < 0.0 )?1:0);
        out += ((dot( frustum_planes[i], glm::vec4(max[0], max[1], min[2], 1.0f) ) < 0.0 )?1:0);
        out += ((dot( frustum_planes[i], glm::vec4(min[0], min[1], max[2], 1.0f) ) < 0.0 )?1:0);
        out += ((dot( frustum_planes[i], glm::vec4(max[0], min[1], max[2], 1.0f) ) < 0.0 )?1:0);
        out += ((dot( frustum_planes[i], glm::vec4(min[0], max[1], max[2], 1.0f) ) < 0.0 )?1:0);
        out += ((dot( frustum_planes[i], glm::vec4(max[0], max[1], max[2], 1.0f) ) < 0.0 )?1:0);
        if( out==8 ) {
            printf("Failing plane: %d\n", i);
            return false;
        }
    }

    // check frustum outside/inside box
    int out;
    out=0; 
    for( int i=0; i<8; i++ ) 
    { 
        out += ((frustum_points[i][0] > max[0])?1:0); 
    } if( out==8 ) return false;
    out=0; for( int i=0; i<8; i++ ) { out += ((frustum_points[i][0] < min[0])?1:0); } if( out==8 ) return false;
    out=0; for( int i=0; i<8; i++ ) { out += ((frustum_points[i][1] > max[1])?1:0); } if( out==8 ) return false;
    out=0; for( int i=0; i<8; i++ ) { out += ((frustum_points[i][1] < min[1])?1:0); } if( out==8 ) return false;
    out=0; for( int i=0; i<8; i++ ) { out += ((frustum_points[i][2] > max[2])?1:0); } if( out==8 ) return false;
    out=0; for( int i=0; i<8; i++ ) { out += ((frustum_points[i][2] < min[2])?1:0); } if( out==8 ) return false;

    float radius = distance(aabb->center, max);
    for (int i = 0; i < 6; ++i)
    {
        int out = 0;
        out += ((dot( frustum_planes[i], glm::vec4(aabb->center, 1.0f) ) < -radius )?1:0);
        if( out==1 ) {
            printf("Failing plane: %d\n", i);
            return false;
        }
    }

    // If either the min/max point was found to be in the Frustum, return
    // true.
    return true;
    // return intersection_min|intersection_max;
}


bool PointBetweenTwoRays(glm::vec3 ray_min, glm::vec3 ray_max, glm::vec3 origin, glm::vec3 point)
{
    glm::vec3 point_ray = normalize(point - origin);

    float m = (ray_min[0] * (-1 * point_ray[2]) + ray_min[2] * point_ray[0]);
    float a = (ray_max[0] * (-1 * point_ray[2]) + ray_max[2] * point_ray[0]);

    return (ray_min[0] * (-1 * point_ray[2]) + ray_min[2] * point_ray[0] >= 0) &&
           (ray_max[0] * (-1 * point_ray[2]) + ray_max[2] * point_ray[0] <= 0);
}