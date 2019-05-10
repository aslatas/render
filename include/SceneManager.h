#ifndef SCENE_MANAGER_H

struct MaterialElement
{
    size_t type_id;
    size_t mat_id;
};

struct MeshElement
{
    size_t filename_idx; // hashed index for filename
    size_t friendly_idx; // hashed index for friendly name     
    Mesh   *mesh;
};

struct HashElement
{
    char *key;
    void *value;
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
    // MaterialLayout  *material_types  = nullptr; // array
    MeshElement        *meshes          = nullptr; // array

    // Seed for the hash table
    size_t seed         = -1; // seed given to hash table
    size_t scene_id     = -1; // hashed id of scene filename (settings)
    size_t bin_scene_id = -1; // hashed id of the binary scene filename (data)

    // Friendly names for the material and mesh
    HashElement *friendly_mat  = nullptr; // key is engine friendly name
                                          // value is MaterialElement, contains 
                                          // index into material array.
    HashElement *friendly_mesh = nullptr; // 

    // Filenames for material and mesh
    HashElement *filename_mat  = nullptr; // Key is the filename,
                                          // value is index into material array. 
                                          // Mesh.materialType gives the material type
    HashElement *filename_mesh = nullptr; // key is the filename, 
                                          // value is MaterialElement, contains index into
                                          // mat type and mat array
    

public:
    MaterialLayout* material_types  = nullptr; // HashTable
    // Model*     model_data = nullptr; // ArrayList
    OctTree* scene = nullptr;
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

    //-----------------------------------------------------------------------//
    // Mesh related functions
    //-----------------------------------------------------------------------//
    // Only gltf is supported
    enum EMeshFileType
    {
        GLTF,
        OBJ,
        FBX
    };
    // Returns the index into the mesh array
    size_t LoadMeshFromFile(EMeshFileType type, char *filename);
    size_t AddMesh(Mesh *mesh, char* filename, char *friendly);
    Mesh *GetMeshByFriendlyName(char *friendly);
    Mesh *GetMeshByFilename(char *filename);
    u32 ChangeMeshFriendlyName(char *old_name, char *new_name);
    u32 ChangeMeshFilename(char *old_name, char *new_name);
    Mesh *GetAllMeshes();

    //-----------------------------------------------------------------------//
    // OctTree related functions
    //-----------------------------------------------------------------------//
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
    RenderSceneMaterial* GetVisibleData(Camera::Camera *camera, glm::vec3 *camera_position = nullptr);


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
