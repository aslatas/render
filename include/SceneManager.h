#ifndef SCENE_MANAGER_H

struct HashModel 
{
    size_t key;    // hashed key 
    void* value; // struct containing the data
};

/*
 * Here is what I want: 
 *   An Array of Key-Object struct
 *     Hashed Key
 *     void* object -> some other struct containing the data
 *   
 *   Return the index of th
 */
struct FakeHashTable
{
    long seed = -1;
    HashModel* hash_table = nullptr;

};

// Probably unnecessary right now, but there is a possibility we use other 
// spatial heirarchies in the future and this can be used to create different
// types of heirarchies
// enum ESpatialHeirarchy
// {
//     OCT_TREE
// };

// Struct that is returned when RenderScene is called. Generally will be returned as a list
// mat_layout_idx: index into MaterialLayoutList to get the MaterialLayout
// mat_idx: index into the material list in a given MaterialLayout 
// scene_models: list of indices into the SceneManager Model list. Each model in this list is attached
//               to the material pipeline
struct scene_mat {
    u32 mat_idx;
    u32 *model_idx;
};
struct RenderSceneMaterial
{
    u32 mat_layout_idx;       // index into SceneManager Material list for this material
    scene_mat *scene_materials; // list of indices into SceneManager Model List that have this material 
};

class SceneManager {

private:
    //---------------//
    // SINGLETON
    //---------------//
    // SceneManager();
    // static SceneManager* manager;

    //---------------//
    // Internal State
    //---------------//
    // Scene Assets
    MaterialLayout* material_types  = nullptr; // HashTable
    FakeHashTable* models     = nullptr; // HashTable
    Model*     model_data = nullptr; // ArrayList

    // Spatial Heirarchies
    // ESpatialHeirarchy scene_active_spatial_heirarchy;
    OctTree* scene = nullptr;

public:
    // Remove the copy constructors
    // SceneManager(const SceneManager&)            = delete;
    // SceneManager& operator=(const SceneManager&) = delete;
    SceneManager();
    ~SceneManager(); // Destructor

    // static const SceneManager* GetInstance();

    // Frees the object's resources
    void Shutdown();

    //--------------------------------------------------------------------------------//
    // Data Load Functions
    //--------------------------------------------------------------------------------//
    // Gets added to the MaterialLayout List and returns the index at which it was
    // added to 
    u32 AddMaterialType(MaterialLayout *layout);
    // Given the index into the MaterialLayout list, add a material to that type. Return
    // The index into the material list that material was added to.
    u32 AddMaterial(Material* mat, u32 layout_idx);
    // Does not really load from a file
    // Note: filename is the hashed key for the HashTable
    size_t LoadModel(char* filename, u32 mat_idx);

    void AddModel(char* key, Model* model);

    // OctTree is not populated by default. Uses the Model HashTable
    // to populate the HashTable.
    void CreateSpatialHeirarchy(float *min, float *max);
    void LoadOctTree();

    //--------------------------------------------------------------------------------//
    // Getters
    //--------------------------------------------------------------------------------//
    size_t GetModelIndex(char* key);
    u32 GetNumberOfModelsInScene();
    Model*    GetModel(char* key);
    Model*    GetModelByIndex(u32 mod_idx);
    HashModel* GetModelStruct(char* key);
    MaterialLayout* GetMaterialLayout(u32 mat_layout_idx);
    Material* GetMaterial(u32 mat_layout_idx, u32 mat_idx);

    //--------------------------------------------------------------------------------//
    // Render functions
    //--------------------------------------------------------------------------------//
    // Performs frustum culling on the OctTree
    void FrustumCull(Camera::Frustum *frustum);
    // Performs occlusion culling on the OctTree
    void OcclusionCullOctTree();
    // Retrive all visible data in the Tree
    RenderSceneMaterial* GetVisibleData(glm::vec3 *camera_position = nullptr);


    //--------------------------------------------------------------------------------//
    // Print Functions
    //--------------------------------------------------------------------------------//
    // Prints the scene 
    // -> using the QuadTree
    // -> Print Contents of the HashTables with filenames unhashed for visuals
    void PrintScene();
    void PrintModelTable();
};

#define SCENE_MANAGER_H
#endif
