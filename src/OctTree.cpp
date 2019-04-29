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
OctTree::Node* OctTree::create_node(AABB_3D* aabb, Node *parent)
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

void OctTree::helper_lazy_occlusion_culling(int position, OcclusionList *list, glm::vec3 *occluder_size)
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
            OctTree::helper_lazy_occlusion_culling((position * TREE_CHILDREN) + i, list, occluder_size); 
        }
    }
    else
    {
        for (int j = 0; j < node->bin->count; ++j)
        {
            SpatialModel sm = *node->bin->model[j];
            
            // glm::vec3 is_occluder = (occluder_size = nullptr) ? sm.aabb.ext : (sm.aabb.ext - *occluder_size); 
            glm::vec3 is_occluder = sm.aabb.ext;
            if (occluder_size != nullptr)
            {
                is_occluder -= *occluder_size;
            }

            // the model ext is smaller than the set occluder ext, so put in the occludee list
            if (is_occluder[0] < 0 && is_occluder[1] < 0 && is_occluder[2] < 0)
            {
                arrput(list->occludee, sm);
            }
            // the model ext is greater than the set occluder ext, so put in the occluder list
            else
            {
                // TODO(Dustin): Check to see if this model occludes other objects in the list or is occluded
                arrput(list->occluder, sm);
            }
        }
    }
}

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
                                                 glm::vec3 *occluder_size,
                                                 ECullingSettings type)
{
    if (arrlen(tree) < 0)
        return nullptr;

    OcclusionList *ol = (OcclusionList*)malloc(sizeof(OcclusionList));
    ol->occluder = nullptr;
    ol->occludee = nullptr;

    OctTree::helper_lazy_occlusion_culling(0, ol, occluder_size);
    
    

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
        // Model occludee_model = models[occluder.model_index];
        

        bool isOccluded = false;
        for (int j = 0; j < arrlen(ol->occluder); ++j)
        {
            SpatialModel occluder = ol->occluder[j];

            // get the model
            

            // used for the other two types of occlusion: mild and aggressive only check if there
            // is a full overlap between AABBs. 
            bool isFullOverlap = false;

            if (type == OCCLUSION_LAZY)
            {
                // Check bounding box overlap
                // glm::vec3 max_ray = (occluder.aabb.max - *camera_position);
                // glm::vec3 point_max_ray = (occludee.aabb.max - *camera_position);

                // // assume the occludee is in front of the occluder
                // if (length(point_max_ray) <= length(max_ray))
                // {
                //     continue;
                // }

                // // now that we have confirmed the occludee is farther back than the 
                // // occluder continue creating rays
                // max_ray = normalize(max_ray);
                // point_max_ray = normalize(point_max_ray);
                // glm::vec3 min_ray = normalize(occluder.aabb.min - *camera_position);
                // glm::vec3 point_min_ray = normalize(occludee.aabb.min - *camera_position);

                // float ray_angle = abs(dot(max_ray, min_ray));
                // float min_ray_angle = abs(dot(min_ray, point_min_ray));
                // float max_ray_angle = abs(dot(min_ray, point_max_ray));

                // if((ray_angle >= 0.99998f && ray_angle <= 1.0f) || 
                //    ((min_ray_angle <= ray_angle && min_ray_angle > 0) && 
                //    (max_ray_angle <= ray_angle && max_ray_angle > 0) ) )
                // {
                //     isOccluded = true;
                //     isFullOverlap = true;
                // } 

                glm::vec3 er_max = occluder.aabb.max;
                glm::vec3 ee_max = occludee.aabb.max;

                glm::vec3 max_ray = (occluder.aabb.max - *camera_position);
                glm::vec3 point_max_ray = (occludee.aabb.max - *camera_position);

                // Z is up, so y is depth
                // TODO(Dustin): what if they are in opposite directions?
                if (abs(max_ray[1]) <= abs(point_max_ray[1]))
                {
                    max_ray = normalize(max_ray);
                    point_max_ray = normalize(point_max_ray);
                    glm::vec3 point_min_ray = normalize(occludee.aabb.min - *camera_position);
                    glm::vec3 min_ray = normalize(occluder.aabb.min - *camera_position);
                    bool retmin = PointBetweenTwoRays(min_ray, max_ray, *camera_position, occludee.aabb.min);
                    bool retmax = PointBetweenTwoRays(min_ray, max_ray, *camera_position, occludee.aabb.max);

                    glm::vec3 er_min = occluder.aabb.min; // A
                    glm::vec3 ee_min = occludee.aabb.min; // B
                    // bool testA = ;

                    bool l=false;
                    bool ll=false;
                    if (l && ll) 
                    {
                        printf("this is stupid!\n");
                    }

                    // between the min/max rays
                    if (retmin && retmax)
                    {
                        /*
                        b = er
                        a = ee
                        (er_min[0] >= ee_min[0] && er_min[1] >= ee_min[1]) &&
                        (er_min[0])
                        */
                        glm::vec3 vis_ray_max = *camera_position + 1.0f * max_ray;
                        glm::vec3 vis_ray_min = *camera_position + 1.0f * min_ray;
                        glm::vec3 vis_box_max = *camera_position + 1.0f * point_max_ray;
                        glm::vec3 vis_box_min = *camera_position + 1.0f * point_min_ray;

                        bool vis_max = vis_ray_max[0] >= vis_box_max[0] && vis_ray_max[2] >= vis_box_max[2];
                        bool vis_min = vis_ray_min[0] <= vis_box_min[0] && vis_ray_min[2] <= vis_box_min[2];

                        
                        if (vis_max && vis_min)
                        {
                            isOccluded = true;
                            isFullOverlap = true;
                        }
                        
                    }

                }

                
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
