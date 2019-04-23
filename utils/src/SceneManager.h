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
    Material*      materials  = nullptr; // HashTable
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
    // Visual only - gets added to the materials list
    size_t LoadMaterial(char* key);
    // Does not really load from a file
    // Note: filename is the hashed key for the HashTable
    size_t LoadModel(char* filename, u32 mat_idx);

    void AddModel(float* min, float* max, size_t mat_index, int val);

    // OctTree is not populated by default. Uses the Model HashTable
    // to populate the HashTable.
    void CreateSpatialHeirarchy(float *min, float *max);
    void LoadOctTree();

    //--------------------------------------------------------------------------------//
    // Getters
    //--------------------------------------------------------------------------------//
    size_t GetModelIndex(char* key);
    Model*    GetModel(char* key);
    HashModel* GetModelStruct(char* key);

    //--------------------------------------------------------------------------------//
    // Render functions
    //--------------------------------------------------------------------------------//
    // Performs frustum culling on the OctTree
    void FrustumCullOctTree();
    // Performs occlusion culling on the OctTree
    void OcclusionCullOctTree();
    // Retrive all visible data in the Tree
    void GetVisibleData();

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
