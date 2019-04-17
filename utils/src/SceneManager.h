#ifndef SCENE_MANAGER_H

struct HashModel 
{
    char* key;
    Model* value;
};

class SceneManager {

private:
    // Scene Assets
    Material*  materials  = nullptr; // HashTable
    HashModel*     models     = nullptr; // HashTable
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
    int LoadMaterial(char* key);
    // Does not really load from a file
    // Note: filename is the hashed key for the HashTable
    int LoadModel(char* filename);
    // OctTree is not populated by default. Uses the Model HashTable
    // to populate the HashTable.
    void LoadOctTree();

    ptrdiff_t GetModelIndex(char* key);
    HashModel GetModelStruct(char* key);

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
};

#define SCENE_MANAGER_H
#endif
