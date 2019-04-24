
/*
 1. If the matrix M is equal to the projection matrix P(i.e., M = P), then the 
    algorithm gives the clipping planesin view space (i.e., camera space).
 
 2. If the  matrixMis  equal  to  the  combined  projection  and  modelview  
    matrices,  then  the  algorithm  gives  theclipping  planes  in  model  space  
    (i.e.,  ⋅=VP  M,  where  Vis  the  modelview  matrix,  and  Pis  the  
    projectionmatrix).
 *
 *
 */
glm::vec4 *ExtractFrustumPlanes(glm::mat4 frustum)
{
    glm::vec4 *planes = (glm::vec4*)malloc(4 * sizeof(glm::vec4));

    // left
    planes[0][0] = frustum[3][0] + frustum[0][0];
    planes[0][1] = frustum[3][1] + frustum[0][1];
    planes[0][2] = frustum[3][2] + frustum[0][2];
    planes[0][3] = frustum[3][3] + frustum[0][3];

    // right
    planes[1][0] = frustum[3][0] - frustum[0][0];
    planes[1][1] = frustum[3][1] - frustum[0][1];
    planes[1][2] = frustum[3][2] - frustum[0][2];
    planes[1][3] = frustum[3][3] - frustum[0][3];

    // bottom
    planes[2][0] = frustum[3][0] + frustum[1][0];
    planes[2][1] = frustum[3][1] + frustum[1][1];
    planes[2][2] = frustum[3][2] + frustum[1][2];
    planes[2][3] = frustum[3][3] + frustum[1][3];
    
    // top
    planes[3][0] = frustum[3][0] - frustum[1][0];
    planes[3][1] = frustum[3][1] - frustum[1][1];
    planes[3][2] = frustum[3][2] - frustum[1][2];
    planes[3][3] = frustum[3][3] - frustum[1][3];

    // near
    planes[2][0] = frustum[3][0] + frustum[2][0];
    planes[2][1] = frustum[3][1] + frustum[2][1];
    planes[2][2] = frustum[3][2] + frustum[2][2];
    planes[2][3] = frustum[3][3] + frustum[2][3];
    
    // far
    planes[3][0] = frustum[3][0] - frustum[2][0];
    planes[3][1] = frustum[3][1] - frustum[2][1];
    planes[3][2] = frustum[3][2] - frustum[2][2];
    planes[3][3] = frustum[3][3] - frustum[2][3];

    // normalize
    // TODO(Dustin): use GLM math
    for (int i = 0; i < 4; ++i)
    {
        float mag = sqrt(planes[i][0] + 
                         planes[i][1] + 
                         planes[i][2] + 
                         planes[i][3]);
        planes[i][0] /= mag;
        planes[i][1] /= mag;
        planes[i][2] /= mag;
        planes[i][3] /= mag;
    }

    return planes;
}

enum HalfSpace
{
    NEGATIVE = 0x00, // outside
    ON_PLANE = 0x01,  // on the plane
    POSITIVE = 0x10   // inside
};

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

bool AABBFrustumIntersection(glm::vec4 *frustum_planes, AABB *aabb)
{
    unsigned char intersection_min = 0x11; // assume inside
    unsigned char intersection_max = 0x11; // assume inside
    for (int i = 0; i < 4; ++i)
    {
        // determine if the point is inside, on, or outside the plane
        // looking for a single case of outside. All points for a min/max
        // should exist in Positive Space or On the plane to be inside the
        // Frustum. Once intersection is locked at 0x00, it stays there.
        intersection_min &= ClassifyPoint(frustum_planes[i], *aabb->min);
        intersection_max &= ClassifyPoint(frustum_planes[i], *aabb->max);
    }

    // If either the min/max point was found to be in the Frustum, return
    // true.
    return intersection_min|instersection_max;
}
