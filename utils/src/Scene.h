#ifndef SCENE_H

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

/*
Array For:
-> MaterialLayout/Materials
-> Meshes

HashTable for:
-> Material "Friendly" name
-> Mesh Filename
-> Mesh "Friendly" name
*/

class Scene
{
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
    Scene(long _seed, char* filepath);
    ~Scene();

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

    //-----------------------------------------------------------------------//
    // Material related functions
    //-----------------------------------------------------------------------//

    //u32 AddMaterial(u32 mat_type, );
};

#define SCENE_H
#endif