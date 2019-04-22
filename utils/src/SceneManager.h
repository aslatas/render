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

class SceneManager {

private:
    // Scene Assets
    Material*  materials  = nullptr; // HashTable
    FakeHashTable*     models     = nullptr; // HashTable
    ModelData* model_data = nullptr; // ArrayList


    // OctTree containg spatial representation of Data
    OctTree* scene;

public:
    SceneManager();  // Constructor
    ~SceneManager(); // Destructor

    //--------------------------------------------------------------------------------//
    // Data Load Functions
    //--------------------------------------------------------------------------------//
    // Visual only - gets added to the materials list
    size_t LoadMaterial(char* key);
    // Does not really load from a file
    // Note: filename is the hashed key for the HashTable
    size_t LoadModel(char* filename, ptrdiff_t mat_idx);
    // OctTree is not populated by default. Uses the Model HashTable
    // to populate the HashTable.
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
