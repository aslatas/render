#include "OctTree.h"

/*
TODOs for the QuadTree
-> Free your memory!!!
-> Play with Bin growth heuristic
-> Convert the Bin contents to a void* so the tree can be used for other objects

-> Memory Allocator for OctTree
  Essentailly the total size of the array containing the octtree is limited by the depth of the 
  tree. This is due to the array representation of the tree, which can cause an integer overflow
  should the array become too large. To solve this problem, a memory allocator can be used. Allocate
  a portion of memory that gets filled up as the tree expands. Now there is a potential problem
  where bins are freed, creating internal fragmentation. It is like this block of memory will have to
  occasionally grow, so during this expansion implement a hueristic that tightens the memory gaps
  produced from freeing Bins.
  This solves the integer overflow at index, I now longer index into the tree based on an int, but 
  rather a pointer. Still maintain as much cache coherence as an array based list with less internal
  fragmentation.
*/

#define TREE_CHILDREN 8
#define INITIAL_BIN_SIZE 10
#define BIN_DEPTH(x) ((x) * 1.4)



//-------------------------------------------------------------------//
// Helper Functions
//-------------------------------------------------------------------//
Node* OctTree::create_node(AABB_3D* aabb, Node *parent)
{
    Node *n = (Node *)malloc(sizeof(Node));
    n->bounding_box = aabb;
    n->parent = parent;
    // Not quite a double, but a slow growth based on depth
    n->bin_size = (parent == nullptr) ? INITIAL_BIN_SIZE : ((u32)(BIN_DEPTH(parent->bin_size)));
    n->isLeaf = true;
    n->isVisible = true;
    n->bin = (Bin *)malloc(sizeof(Bin));
    n->bin->count = 0;
    n->bin->model = nullptr;
    
    return n;
}

bool OctTree::helper_add(int position, SpatialModel *model)
{
    Node *node = tree[position];
    if (!node->isLeaf) // this node has children, need to go further down the tree
    {
        bool at_least_one_node_intersection_found = false;
        for (int i = 0; i < TREE_CHILDREN; ++i)
        {
            assert((position * TREE_CHILDREN) + (i + 1) < arrlen(tree)); // the child node should be within bounds of the tree
            if (CheckBoundingBoxCollision3D(model->aabb, *tree[(position * TREE_CHILDREN) + (i + 1)]->bounding_box))
            {
                at_least_one_node_intersection_found = true;
                helper_add((position * TREE_CHILDREN) + (i + 1), model);
            }
        }
        
        return at_least_one_node_intersection_found;
    }
    else // we are at a leaf
    {
        assert(node->bin); // the bin should be allocated
        if ((u32)node->bin->count + 1 > node->bin_size)
        {
            // split and then re-add to this node, which is not a leaf
            split(position);
            helper_add(position, model);
        }
        else
        {
            arrput(node->bin->model, model);
            ++node->bin->count;
        }
        
        return true;
    }
}

void OctTree::split(int position)
{
    printf("SPLITTT\n");
    Node *node = tree[position];
    
    AABB_3D *aabb = node->bounding_box;
    glm::vec3   min  = aabb->min;
    glm::vec3   max  = aabb->max;
    glm::vec3   center  = aabb->center;

    // half the distance between min and max
    // float ext[3] = {(max[0] - min[0]) / 2, (max[1] - min[1]) / 2, (max[2] - min[2]) / 2};
    
    // Create the AABBs of the children of this node 
    // front top left
    float ftl_min[3] = {min[0], center[1], center[2]};
    float ftl_max[3] = {center[0], max[1], max[2]};
    AABB_3D *ftl = Create3DAxisAlignedBoundingBox(ftl_min, ftl_max);
    
    // front bottom left
    float fbl_min[3] = {min[0], min[1], center[2]};
    float fbl_max[3] = {center[0], center[1], max[2]};
    AABB_3D *fbl = Create3DAxisAlignedBoundingBox(fbl_min, fbl_max);

    // front top right
    float ftr_min[3] = {center[0], center[1], center[2]};
    float ftr_max[3] = {max[0], max[1], max[2]};
    AABB_3D *ftr = Create3DAxisAlignedBoundingBox(ftr_min, ftr_max);

    // front bottom right
    float fbr_min[3] = {center[0], min[0], center[2]};
    float fbr_max[3] = {max[0], center[1], max[2]};
    AABB_3D *fbr = Create3DAxisAlignedBoundingBox(fbr_min, fbr_max);

    // back top left
    float btl_min[3] = {min[0], center[1], min[2]};
    float btl_max[3] = {center[0], max[1], center[2]};
    AABB_3D *btl = Create3DAxisAlignedBoundingBox(btl_min, btl_max);

    // back bottom left
    float bbl_min[3] = {min[0], min[1], min[2]};
    float bbl_max[3] = {center[0], center[1], center[2]};
    AABB_3D *bbl = Create3DAxisAlignedBoundingBox(bbl_min, bbl_max);

    // back top right
    float btr_min[3] = {center[0], center[1], min[2]};
    float btr_max[3] = {max[0], max[1], center[2]};
    AABB_3D *btr = Create3DAxisAlignedBoundingBox(btr_min, btr_max);

    // back bottom right
    float bbr_min[3] = {center[0], min[1], min[2]};
    float bbr_max[3] = {max[0], center[1], center[2]};
    AABB_3D *bbr = Create3DAxisAlignedBoundingBox(bbr_min, bbr_max);

    // grow if necessary
    if ((position * TREE_CHILDREN) + TREE_CHILDREN >= arrlen(tree))
    {
        arrsetlen(tree, ((u32)arrlen(tree) + 1) * TREE_CHILDREN);
    }
    
    // Create nodes for childre
    tree[(position * TREE_CHILDREN) + 1] = create_node(ftl, node);
    tree[(position * TREE_CHILDREN) + 2] = create_node(fbl, node);
    tree[(position * TREE_CHILDREN) + 3] = create_node(ftr, node);
    tree[(position * TREE_CHILDREN) + 4] = create_node(fbr, node);
    tree[(position * TREE_CHILDREN) + 5] = create_node(btl, node);
    tree[(position * TREE_CHILDREN) + 6] = create_node(bbl, node);
    tree[(position * TREE_CHILDREN) + 7] = create_node(btr, node);
    tree[(position * TREE_CHILDREN) + 8] = create_node(bbr, node);
    
    // Now an internal node
    node->isLeaf = false;
    
    // Re-add the contents of this Bin to the node so they are added to the approppriate child
    for (int i = 0; i < node->bin->count; ++i)
    {
        SpatialModel *model = node->bin->model[i];
        helper_add(position, model);
    }
    
    // No longer should have a bin attached to the node
    arrfree(node->bin->model);
    free(node->bin);
    node->bin = nullptr;
}

void OctTree::helper_print_quad_tree(int position)
{
    Node *node = tree[position];
    
    printf("Node %d at level %d.\n", position, position / 4);
    printf("This node has this bounding box.\n");
    printf("  Bounding Box:\n      Minimum: (%f, %f, %f)\n      Maximum: (%f, %f, %f)\n      Center: (%f, %f, %f)\n      Extent: (%f, %f, %f)\n", 
        node->bounding_box->min[0], node->bounding_box->min[1], node->bounding_box->min[2], 
        node->bounding_box->max[0], node->bounding_box->max[1], node->bounding_box->max[2], 
        node->bounding_box->center[0], node->bounding_box->center[1], node->bounding_box->center[2], 
        node->bounding_box->ext[0], node->bounding_box->ext[1], node->bounding_box->ext[2]);
    
    if (!node->parent)
    {
        printf("  This node is the root.\n");
    }
    if (!node->isLeaf)
    {
        printf("  This node has the following children at the indices:\n");
        for (int i = 1; i <= TREE_CHILDREN; ++i) {
            helper_print_quad_tree((position * TREE_CHILDREN) + i);    
        }
    }
    else
    {
        printf("  This node has no children.\n");
    }
    
    if (!node->bin)
    {
        printf("  This node does not have a bin.\n");
    }
    else
    {
        if (node->bin->count == 0)
        {
            printf("  This node has a bin, but contains no models.\n");
        }
        else
        {
            printf("  Bin Size: %d\n", node->bin_size);    
            printf("  This node's bin contains %d models:\n", node->bin->count);
            for (int j = 0; j < node->bin->count; ++j)
            {
                SpatialModel *m = node->bin->model[j];
                printf("    Bounding Box:\n      Minimum: (%f, %f, %f)\n      Maximum: (%f, %f, %f)\n      Center: (%f, %f, %f)\n      Extent: (%f, %f, %f)\n", 
                    m->aabb.min[0], m->aabb.min[1], m->aabb.min[2],
                    m->aabb.max[0], m->aabb.max[1], m->aabb.max[2],
                    m->aabb.center[0], m->aabb.center[1], m->aabb.center[2], 
                    m->aabb.ext[0], m->aabb.ext[1], m->aabb.ext[2]);
                // printf("    Data: %d\n", m->val);
            }
            //printf("  ");
        }
    }
    printf("\n");
}

void OctTree::helper_frustum_visibility(int position, Camera::Frustum *frustum)
{
    Node *node = tree[position];
    node->isVisible = AABBFrustumIntersection(frustum, node->bounding_box);
    if (node->isVisible)
    {

        if (!node->isLeaf) {
            // recursively search for a leaf
            for (int i = 1; i <= TREE_CHILDREN; ++i) {
                OctTree::helper_frustum_visibility((position * TREE_CHILDREN) + i, frustum); 
            }
        }
        else
        {
            for (int j = 0; j < node->bin->count; ++j)
            {
                node->bin->model[j]->isVisible = AABBFrustumIntersection(frustum, &node->bin->model[j]->aabb);
            }
        }
    }
}

void OctTree::helper_lazy_occlusion_culling(int position, OcclusionList *list, Camera::Camera *camera, glm::vec3 *occluder_size)
{
    Node *node = tree[position];

    // don't evaluate a branch that is not visible
    if (!node->isVisible)
        return;

    // don't care about non-branches
    if (!node->isLeaf)
    {
        // recursively search for a leaf
        for (int i = 1; i <= TREE_CHILDREN; ++i) {
            OctTree::helper_lazy_occlusion_culling((position * TREE_CHILDREN) + i, list, camera, occluder_size); 
        }
    }
    else
    {
        for (int j = 0; j < node->bin->count; ++j)
        {
            SpatialModel sm = *node->bin->model[j];
            if (!sm.isVisible) continue;

            // determine screen size
            glm::mat4 cam = GetViewTransform(camera);
            glm::mat4 proj = GetProjectionTransform(camera);

            glm::vec4 min_screen = cam * glm::vec4(sm.aabb.min, 1.0f);
            glm::vec4 max_screen = cam * glm::vec4(sm.aabb.max, 1.0f);
            min_screen /= min_screen.w;
            max_screen /= max_screen.w;

            min_screen = proj * min_screen;
            max_screen = proj * max_screen;
            min_screen /= min_screen.w;
            max_screen /= max_screen.w;

            min_screen[0] = (fmin(fmax(-1.0f, min_screen[0]), 1.0f));
            min_screen[1] = (fmin(fmax(-1.0f, min_screen[1]), 1.0f));
            min_screen[2] = (fmin(fmax(-1.0f, min_screen[2]), 1.0f));

            max_screen[0] = (fmin(fmax(-1.0f, max_screen[0]), 1.0f));
            max_screen[1] = (fmin(fmax(-1.0f, max_screen[1]), 1.0f));
            max_screen[2] = (fmin(fmax(-1.0f, max_screen[2]), 1.0f));

            float distx = abs(max_screen[0] - min_screen[0]);
            float disty = abs(max_screen[1] - min_screen[1]);
            float distz = abs(max_screen[2] - min_screen[2]);
            float ext = distx * disty;
            if (ext >= -0.00001 && ext <= 0.00001) continue;

            glm::vec3 loc = camera->location;
            float dist = distance(loc, sm.aabb.max);
            dist = fmin(dist, distance(loc, sm.aabb.max));
            // bool found = false;
            // for (int i = 0; i < arrlen(list->occludee); ++i)
            // {
            //     SpatialModel sme = list->occludee[i];
            //     float exist_dist = fmin(distance(loc, sme.aabb.max), distance(loc, sme.aabb.max));
            //     if (dist < exist_dist)
            //     {
            //         found = true;
            //         arrins(list->occludee, i, sm);
            //         brea
            //     }
            // }

            // if (!found)
            // {
            //     arrput(list->occludee, sm);
            // }

            if (ext > 1.0f)
            {
                arrput(list->occludee, sm);
            }
            else
            {
                arrput(list->occluder, sm);
            }

        }
    }
}

// void OctTree::helper_lazy_occlusion_culling(int position, OcclusionList *list, Camera::Camera *camera, glm::vec3 *occluder_size)
// {
//     Node *node = tree[position];

//     // don't evaluate a branch that is not visible
//     if (!node->isVisible)
//         return;

//     // don't care about non-branches
//     if (!node->isLeaf)
//     {
//         // recursively search for a leaf
//         for (int i = 1; i <= TREE_CHILDREN; ++i) {
//             OctTree::helper_lazy_occlusion_culling((position * TREE_CHILDREN) + i, list, camera, occluder_size); 
//         }
//     }
//     else
//     {
//         for (int j = 0; j < node->bin->count; ++j)
//         {
//             SpatialModel sm = *node->bin->model[j];
//             if (!sm.isVisible) continue;
            
//             // glm::vec3 is_occluder = (occluder_size = nullptr) ? sm.aabb.ext : (sm.aabb.ext - *occluder_size); 
//             glm::vec3 is_occluder = sm.aabb.ext;
//             if (occluder_size != nullptr)
//             {
//                 is_occluder -= *occluder_size;
//             }

//             glm::mat4 screen = GetProjectionTransform(camera) * GetViewTransform(camera);
//             // screen = screen / screen[3][3];
//             // screen[3][3] = 1.0f;

//             glm::vec4 min_screen = glm::vec4(sm.aabb.min, 1.0f) * screen;
//             glm::vec4 max_screen = screen * glm::vec4(sm.aabb.max, 1.0f);

//             min_screen /= min_screen[3];
//             max_screen /= max_screen[3];

//             min_screen[0] = (fmin(fmax(-1.0f, min_screen[0]), 1.0f) + 1) / 2;
//             min_screen[1] = (fmin(fmax(-1.0f, min_screen[1]), 1.0f) + 1) / 2;
//             min_screen[2] = (fmin(fmax(-1.0f, min_screen[2]), 1.0f) + 1) / 2;

//             max_screen[0] = (fmin(fmax(-1.0f, max_screen[0]), 1.0f) + 1) / 2;
//             max_screen[1] = (fmin(fmax(-1.0f, max_screen[1]), 1.0f) + 1) / 2;
//             max_screen[2] = (fmin(fmax(-1.0f, max_screen[2]), 1.0f) + 1) / 2;

//             float distx = abs(max_screen[0] - min_screen[0]);
//             float disty = abs(max_screen[1] - min_screen[1]);
//             float distz = abs(max_screen[2] - min_screen[2]);
//             float ext = distx * disty;
//             if (ext >= -0.001 && ext <= 0.001) continue;

//             // glm::vec3 ext = max_screen - min_screen;
//             // float ext = sqrt((max_screen[0] - min_screen[0]) * (max_screen[0] - min_screen[0]) + 
//             //     // (max_screen[1] - min_screen[1]) * (max_screen[1] - min_screen[1]) +
//             //     (max_screen[1] - min_screen[1]) * (max_screen[1] - min_screen[1]));
//             // float percent_screen = ext / 2.0f;
//             // glm::vec3 percent_screen = abs(ext) / 2.0f;

//             glm::vec4 op_occ_max = screen * glm::vec4(sm.aabb.max, 1.0f); 
//             op_occ_max /= op_occ_max[3];

//             // the model ext is smaller than the set occluder ext, so put in the occludee list
//             // printf("give me output nowwwww\n");
//             // printf("Ext: %lf\n", ext);`
//             if (ext < 0.3f)
//             {
//                 bool isFullyOccluded = false;
//                 // min_screen = glm::vec4(sm.aabb.min, 1.0f) * screen;
//                 // max_screen = screen * glm::vec4(sm.aabb.max, 1.0f);

//                 // min_screen /= min_screen[3];
//                 // max_screen /= max_screen[3];
//                 // osm_max /= osm_max[3];
//                 // osm_min /= osm_min[3];

//                     // if (osm_min[0] > 1.1 && osm_max[0] > 1.1 || (osm_min[0] < -1.1 && osm_max[0] < -1.1))
//                     // {
//                     //     isFullyOccluded = true;
//                     //     // break;
//                     // }
//                     // if ((osm_min[1] > 1 && osm_max[1] > 1) || (osm_min[1] < -1 && osm_max[1] < -1))
//                     // {
//                     //     isFullyOccluded = true;
//                     //     // break;
//                     // }
//                     // if (osm_min[2] > 1 && osm_max[2] > 1 || (osm_min[2] < -1 && osm_max[2] < -1))
//                     // {
//                     //     isFullyOccluded = true;
//                     //     // break;
//                     // } 
//                 if (!isFullyOccluded) arrput(list->occludee, sm);
//             }
//             // the model ext is greater than the set occluder ext, so put in the occluder list
//             else
//             {
//                 // TODO(Dustin): Check to see if this model occludes other objects in the list or is occluded
//                 bool isFullyOccluded = false;
//                 int idx = 0;
//                 u32 len = arrlen(list->occluder);
//                 for (int i = 0; i < len; ++i)
//                 {
//                     SpatialModel p_occ = list->occluder[idx];

//                     // glm::vec3 er_max = p_occ.aabb.max; // potential occluder/occludee
//                     // glm::vec3 ee_max = sm.aabb.max;    // object attemtping to add

//                     glm::vec3 *occluder_max;
//                     glm::vec3 *occluder_min;
//                     glm::vec3 *occludee_max;
//                     glm::vec3 *occludee_min;

//                     // if p_occ is closer to the camera - it is the occluder
//                     bool add_p_occ_back = true;
//                     glm::vec3 vp = abs((p_occ.aabb.max - camera->location));
//                     glm::vec3 sp = abs((sm.aabb.max - camera->location));
// /*
// glm::vec3 adj_max_ray = glm::vec3(screen * glm::vec4(max_ray, 1.0f));
//                         glm::vec3 adj_min_ray = glm::vec3(screen * glm::vec4(min_ray, 1.0f));
//                         glm::vec3 adj_point_max_ray = glm::vec3(screen * glm::vec4(point_max_ray, 1.0f));
//                         glm::vec3 adj_point_min_ray = glm::vec3(screen * glm::vec4(point_min_ray, 1.0f));

// */
                    
//                     // glm::vec4 op_occ_min = glm::vec3(screen * glm::vec4(sm.aabb.min, 1.0f)); 
//                     // glm::vec4 osm_max = screen * glm::vec4(sm.aabb.max, 1.0f); 
//                     // glm::vec4 osm_min = screen * glm::vec4(sm.aabb.min, 1.0f);
//                     // osm_max /= (osm_max[3] != 0.0f) ? osm_max[3] : 1.0f;
//                     // osm_min /= (osm_min[3] != 0.0f) ? osm_min[3] : 1.0f;
//                     // if (osm_min[0] > 1 && osm_max[0] > 1 || (osm_min[0] < -1 && osm_max[0] < -1))
//                     // {
//                     //     isFullyOccluded = true;
//                     //     break;
//                     // }
//                     // if ((osm_min[1] > 1 && osm_max[1] > 1) || (osm_min[1] < -1 && osm_max[1] < -1))
//                     // {
//                     //     isFullyOccluded = true;
//                     //     break;
//                     // }
//                     // if (osm_min[2] > 1 && osm_max[2] > 1 || (osm_min[2] < -1 && osm_max[2] < -1))
//                     // {
//                     //     isFullyOccluded = true;
//                     //     break;
//                     // }

//                     // glm::vec3 ocamera_p = glm::vec3(screen * glm::vec4(camera->location, 1.0f)); 
//                     glm::vec3 p_occ_max = glm::vec3(glm::vec4(p_occ.aabb.max, 1.0f)); 
//                     glm::vec3 p_occ_min = glm::vec3(glm::vec4(p_occ.aabb.min, 1.0f)); 
//                     glm::vec3 sm_max = glm::vec3(glm::vec4(sm.aabb.max, 1.0f)); 
//                     glm::vec3 sm_min = glm::vec3(glm::vec4(sm.aabb.min, 1.0f)); 
//                     glm::vec3 camera_p = glm::vec3(glm::vec4(camera->location, 1.0f)); 

//                     float depth_p_occ = abs(p_occ_max - camera_p)[1];
//                     float depth_sm = abs(sm_max - camera_p)[1];

//                     if (depth_p_occ > depth_sm)
//                     {
//                         add_p_occ_back = false;

//                         occluder_max = &sm_max;
//                         occluder_min = &sm_min;
                        
//                         occludee_max = &p_occ_max;
//                         occludee_min = &p_occ_min;
//                     }
//                     else 
//                     {
//                         occluder_max = &p_occ_max;
//                         occluder_min = &p_occ_min;
                        
//                         occludee_max = &sm_max;
//                         occludee_min = &sm_min;
//                         // occluder = &p_occ;
//                         // occludee = &sm;
//                     }

//                     // glm::vec3 max_ray = (occluder->aabb.max - camera->location);
//                     // glm::vec3 point_max_ray = (occludee->aabb.max - camera->location);

//                     glm::vec3 max_ray = normalize(*occluder_max - camera_p);
//                     glm::vec3 min_ray = normalize(*occluder_min - camera_p);
//                     glm::vec3 point_max_ray = normalize(*occludee_max - camera_p);
//                     glm::vec3 point_min_ray = normalize(*occludee_min - camera_p);
                    

//                     // between the min/max rays
//                     bool retmin = PointBetweenTwoRays(min_ray, max_ray, camera_p, *occludee_min);
//                     bool retmax = PointBetweenTwoRays(min_ray, max_ray, camera_p, *occludee_max);
//                     if (retmin && retmax)
//                     {
//                             glm::vec3 vis_ray_max = camera_p + 1.0f * max_ray;
//                             glm::vec3 vis_ray_min = camera_p + 1.0f * min_ray;
//                             glm::vec3 vis_box_max = camera_p + 1.0f * point_max_ray;
//                             glm::vec3 vis_box_min = camera_p + 1.0f * point_min_ray;

//                             // Fully occluded
//                             bool vis_max = vis_ray_max[0] >= vis_box_max[0] && vis_ray_max[2] >= vis_box_max[2];
//                             bool vis_min = vis_ray_min[0] <= vis_box_min[0] && vis_ray_min[2] <= vis_box_min[2];
//                             if (vis_max && vis_min)
//                             {
//                                 isFullyOccluded = true;
//                                 // isFullOverlap = true;

//                                 // test element was the occluder, so add it back
//                                 // and make sure the new element is not added
//                                 if (add_p_occ_back)
//                                 {
//                                     // arrput(list->occluder, p_occ);
//                                     break;
//                                 }
//                             }
//                             else 
//                             {
//                                 add_p_occ_back = true;
//                             }
//                     }
//                     else 
//                     {
//                         add_p_occ_back = true;
//                     }
//                     // }

//                     // p_occ was not occluded so add it back to the list
//                     if (!add_p_occ_back) 
//                     {
//                         arrdel(list->occluder, idx);
//                         // arrput(list->occluder, p_occ);
//                     }
//                     else 
//                     {
//                         ++idx;
//                     }
//                 }

//                 if (!isFullyOccluded)
//                     arrput(list->occluder, sm);
//             }
//         }
//     }
// }

SpatialModel* OctTree::helper_get_all_visible_data(SpatialModel* list, int position)
{
    Node *node = tree[position];
    if (!node->isVisible)
    {
        return list;
    }
    else if (!node->isLeaf)
    {
        // recursively search for a leaf
        for (int i = 1; i <= TREE_CHILDREN; ++i) {
            list =  helper_get_all_visible_data(list, (position * TREE_CHILDREN) + i); 
        }
        return list;
    }
    else
    {
        // put each SpatialModel from the Bin into the list
        for (int j = 0; j < node->bin->count; ++j)
        {
            if (node->bin->model[j]->isVisible)
            {
                arrput(list, *node->bin->model[j]);
            }
        }
        return list;
    }
}


//-------------------------------------------------------------------//
// Main Functions
//-------------------------------------------------------------------//
OctTree::OctTree(float* min, float* max)
{
    // First set defaults
    // size_element_per_bin = 0; // keep? Helps to determine the required space for a bin element
    // current_max_depth = 0; // depth of the tree, default is 0
    // bin_size = INITIAL_BIN_SIZE;

    // array representation of the tree. Must be set to null to the stb library
    tree = nullptr; 
    arrsetlen(tree, 1);
    tree[0] = create_node(Create3DAxisAlignedBoundingBox(min, max), nullptr);
}

OctTree::~OctTree()
{

}

void OctTree::Shutdown()
{

}

void OctTree::Print()
{
    helper_print_quad_tree(0);
}

bool OctTree::Add(SpatialModel* model)
{
    if (tree == nullptr) 
    {
        printf("Attempted to add, but tree was null!\n");
        printf("    Bounding Box:\n      Minimum: (%f, %f, %f)\n      Maximum: (%f, %f, %f)\n      Center: (%f, %f, %f)\n      Extent: (%f, %f, %f)\n", 
                    model->aabb.min[0], model->aabb.min[1], model->aabb.min[2],
                    model->aabb.max[0], model->aabb.max[1], model->aabb.max[2],
                    model->aabb.center[0], model->aabb.center[1], model->aabb.center[2], 
                    model->aabb.ext[0], model->aabb.ext[1], model->aabb.ext[2]);
        return false;
    }
    else if (!CheckBoundingBoxCollision3D(*tree[0]->bounding_box, model->aabb))
    {
        printf("Attempted to add, but model was out of bounds!\n");
        return false;
    }
    else {
        bool ret = helper_add(0, model);
        if (!ret)
        {
            printf("failed to add model!\n");
        } 
        return ret;
    }
}

void OctTree::UpdateFrustumVisibility(Camera::Frustum *frustum)
{
    if (arrlen(tree) < 0)
        return;
    OctTree::helper_frustum_visibility(0, frustum);
}

SpatialModel* OctTree::UpdateOcclusionVisibility( glm::vec3 *camera_position,
                                                 Camera::Camera *camera,
                                                 glm::vec3 *occluder_size,
                                                 ECullingSettings type)
{
    if (arrlen(tree) < 0)
        return nullptr;

    OcclusionList *ol = (OcclusionList*)malloc(sizeof(OcclusionList));
    ol->occluder = nullptr;
    ol->occludee = nullptr;

    OctTree::helper_lazy_occlusion_culling(0, ol, camera, occluder_size);
    
    

    // Sort the occluder list by distance from camera (?) - not right now, would have to implement a merge sort

    // (ray_min[0] * (-1 * point_ray[1]) + ray_min[1] * point_ray[0] >= 0) &&
    // (ray_max[0] * (-1 * point_ray[1]) + ray_max[1] * point_ray[0] <= 0);
    // glm::vec3 camera_position = camera.location;

    // if (arrlen(ol->occluder) == 0)
    // {

    // }
    // else if ()
    // {

    // } 

    // Check for occlusion
    SpatialModel *visible_models = nullptr;
    printf("There are %td occluders\n", arrlen(ol->occluder));
    printf("There are %td occludees\n", arrlen(ol->occludee));
    for (int i = 0; i < arrlen(ol->occludee); ++i)
    {
        SpatialModel occludee = ol->occludee[i];

        // glm::mat4 oem = occludee.aabb.max * camera->GetViewTransform(camera);
        // Model occludee_model = models[

/*

 glm::vec4(min[0], min[1], min[2], 1.0f) ) < 0.0 )?1:0);
        out += ((dot( frustum_planes[i], glm::vec4(max[0], min[1], min[2], 1.0f) ) < 0.0 )?1:0);
        out += ((dot( frustum_planes[i], glm::vec4(min[0], max[1], min[2], 1.0f) ) < 0.0 )?1:0);
        out += ((dot( frustum_planes[i], glm::vec4(max[0], max[1], min[2], 1.0f) ) < 0.0 )?1:0);
        out += ((dot( frustum_planes[i], glm::vec4(min[0], min[1], max[2], 1.0f) ) < 0.0 )?1:0);
        out += ((dot( frustum_planes[i], glm::vec4(max[0], min[1], max[2], 1.0f) ) < 0.0 )?1:0);
        out += ((dot( frustum_planes[i], glm::vec4(min[0], max[1], max[2], 1.0f) ) < 0.0 )?1:0);
        out += ((dot( frustum_planes[i], glm::vec4(max[0], max[1], max[2], 1.0f) ) < 0.0 )?1:0);

*/

        glm::vec3 min = occludee.aabb.min;
        glm::vec3 max = occludee.aabb.max;
                glm::vec3 loc = camera->location;
                glm::vec3 ftl = glm::vec3(occludee.aabb.center.x - occludee.aabb.ext.x, occludee.aabb.center.y + occludee.aabb.ext.y, occludee.aabb.center.z + occludee.aabb.ext.z);
                glm::vec3 ftr = glm::vec3(occludee.aabb.center.x + occludee.aabb.ext.x, occludee.aabb.center.y + occludee.aabb.ext.y, occludee.aabb.center.z + occludee.aabb.ext.z);
                glm::vec3 fbl = glm::vec3(occludee.aabb.center.x - occludee.aabb.ext.x, occludee.aabb.center.y + occludee.aabb.ext.y, occludee.aabb.center.z - occludee.aabb.ext.z);
                glm::vec3 fbr = glm::vec3(occludee.aabb.center.x + occludee.aabb.ext.x, occludee.aabb.center.y + occludee.aabb.ext.y, occludee.aabb.center.z - occludee.aabb.ext.z);
                glm::vec3 btl = glm::vec3(occludee.aabb.center.x - occludee.aabb.ext.x, occludee.aabb.center.y - occludee.aabb.ext.y, occludee.aabb.center.z + occludee.aabb.ext.z);
                glm::vec3 btr = glm::vec3(occludee.aabb.center.x + occludee.aabb.ext.x, occludee.aabb.center.y - occludee.aabb.ext.y, occludee.aabb.center.z + occludee.aabb.ext.z);
                glm::vec3 bbl = glm::vec3(occludee.aabb.center.x - occludee.aabb.ext.x, occludee.aabb.center.y - occludee.aabb.ext.y, occludee.aabb.center.z - occludee.aabb.ext.z);
                glm::vec3 bbr = glm::vec3(occludee.aabb.center.x + occludee.aabb.ext.x, occludee.aabb.center.y - occludee.aabb.ext.y, occludee.aabb.center.z - occludee.aabb.ext.z);
                Ray rftl = CreateRay(loc, ftl - loc, 1000.0f);
                Ray rftr = CreateRay(loc, ftr - loc, 1000.0f);
                Ray rfbl = CreateRay(loc, fbl - loc, 1000.0f);
                Ray rfbr = CreateRay(loc, fbr - loc, 1000.0f);
                Ray rbtl = CreateRay(loc, btl - loc, 1000.0f);
                Ray rbtr = CreateRay(loc, btr - loc, 1000.0f);
                Ray rbbl = CreateRay(loc, bbl - loc, 1000.0f);
                Ray rbbr = CreateRay(loc, bbr - loc, 1000.0f);
                Ray rcenter = CreateRay(loc, occludee.aabb.center - loc, 1000.0f);

                bool is_occ[9] = { false };
                int count = 0;
        

        bool isOccluded = false;
        for (int j = 0; j < arrlen(ol->occluder); ++j)
        {
            SpatialModel occluder = ol->occluder[j];

            // used for the other two types of occlusion: mild and aggressive only check if there
            // is a full overlap between AABBs. 
            bool isFullOverlap = false;

            if (type == OCCLUSION_LAZY)
            {
                glm::vec3 i;
                bool ret = false;
                if (!is_occ[0]) is_occ[0] = RayIntersectAxisAlignedBox(rftl, occluder.aabb, &i); if (is_occ[0]) ++count;
                if (!is_occ[1]) is_occ[1] = RayIntersectAxisAlignedBox(rftr, occluder.aabb, &i); if (is_occ[1]) ++count;
                if (!is_occ[2]) is_occ[2] = RayIntersectAxisAlignedBox(rfbl, occluder.aabb, &i); if (is_occ[2]) ++count;
                if (!is_occ[3]) is_occ[3] = RayIntersectAxisAlignedBox(rfbr, occluder.aabb, &i); if (is_occ[3]) ++count;
                if (!is_occ[4]) is_occ[4] = RayIntersectAxisAlignedBox(rbtl, occluder.aabb, &i); if (is_occ[4]) ++count;
                if (!is_occ[5]) is_occ[5] = RayIntersectAxisAlignedBox(rbtr, occluder.aabb, &i); if (is_occ[5]) ++count;
                if (!is_occ[6]) is_occ[6] = RayIntersectAxisAlignedBox(rbbl, occluder.aabb, &i); if (is_occ[6]) ++count;
                if (!is_occ[7]) is_occ[7] = RayIntersectAxisAlignedBox(rbbr, occluder.aabb, &i); if (is_occ[7]) ++count;
                if (!is_occ[8]) is_occ[8] = RayIntersectAxisAlignedBox(rcenter, occluder.aabb, &i); if (is_occ[8]) ++count;

                if (count == 9) 
                {
                    printf("Occludee Mode;:\n");
                    printf("    Bounding Box:\n      Minimum: (%f, %f, %f)\n      Maximum: (%f, %f, %f)\n      Center: (%f, %f, %f)\n      Extent: (%f, %f, %f)\n", 
                    occludee.aabb.min[0], occludee.aabb.min[1], occludee.aabb.min[2],
                    occludee.aabb.max[0], occludee.aabb.max[1], occludee.aabb.max[2],
                    occludee.aabb.center[0], occludee.aabb.center[1], occludee.aabb.center[2], 
                    occludee.aabb.ext[0], occludee.aabb.ext[1], occludee.aabb.ext[2]);

                    printf("Occluder Model:\n");
                    printf("    Bounding Box:\n      Minimum: (%f, %f, %f)\n      Maximum: (%f, %f, %f)\n      Center: (%f, %f, %f)\n      Extent: (%f, %f, %f)\n", 
                    occluder.aabb.min[0], occluder.aabb.min[1], occluder.aabb.min[2],
                    occluder.aabb.max[0], occluder.aabb.max[1], occluder.aabb.max[2],
                    occluder.aabb.center[0], occluder.aabb.center[1], occluder.aabb.center[2], 
                    occluder.aabb.ext[0], occluder.aabb.ext[1], occluder.aabb.ext[2]);
                    isOccluded = true;
                    break;
                }

                // glm::vec3 er_max = occluder.aabb.max;
                // glm::vec3 ee_max = occludee.aabb.max;

                // glm::vec3 max_ray = (occluder.aabb.max - *camera_position);
                // glm::vec3 point_max_ray = (occludee.aabb.max - *camera_position);

                // // Z is up, so y is depth
                // // TODO(Dustin): what if they are in opposite directions?
                // if (abs(max_ray[1]) <= abs(point_max_ray[1]))
                // {
                //     max_ray = normalize(max_ray);
                //     point_max_ray = normalize(point_max_ray);
                //     glm::vec3 point_min_ray = normalize(occludee.aabb.min - *camera_position);
                //     glm::vec3 min_ray = normalize(occluder.aabb.min - *camera_position);

                //     // between the min/max rays
                //     bool retmin = PointBetweenTwoRays(min_ray, max_ray, *camera_position, occludee.aabb.min);
                //     bool retmax = PointBetweenTwoRays(min_ray, max_ray, *camera_position, occludee.aabb.max);
                //     if (retmin && retmax)
                //     {
                //         glm::vec3 vis_ray_max = *camera_position + 1.0f * max_ray;
                //         glm::vec3 vis_ray_min = *camera_position + 1.0f * min_ray;
                //         glm::vec3 vis_box_max = *camera_position + 1.0f * point_max_ray;
                //         glm::vec3 vis_box_min = *camera_position + 1.0f * point_min_ray;

                //         // Fully occluded
                //         bool vis_max = vis_ray_max[0] >= vis_box_max[0] && vis_ray_max[2] >= vis_box_max[2];
                //         bool vis_min = vis_ray_min[0] <= vis_box_min[0] && vis_ray_min[2] <= vis_box_min[2];
                //         if (vis_max && vis_min)
                //         {
                //             isOccluded = true;
                //             isFullOverlap = true;
                //         }
                //     }
                // }
            }

            // it is the responsibility of these two functions to change isOccluded
            // back to false
            if (type == OCCLUSION_MILD && isFullOverlap)
            {

            }

            if (type == OCCLUSION_AGGRESSIVE && isFullOverlap)
            {

            }
            
            if (isOccluded)
                break;
        }

        if (!isOccluded)
        {
            arrput(visible_models, occludee);
        }
    }

    for (int i = 0; i < arrlen(ol->occluder); ++i)
    {
        arrput(visible_models, ol->occluder[i]);
    }

    return visible_models;
}

SpatialModel* OctTree::GetAllVisibleData()
{
    SpatialModel* list = nullptr;
    if (arrlen(tree) > 0)
    {
        return helper_get_all_visible_data(list, 0);
    }
    return list;
}
