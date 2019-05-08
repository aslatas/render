
#include "Scene.h"

Scene::Scene(long _seed, char* filepath)
{
    // MaterialLayout  *material_types  = nullptr; 
    meshes        = nullptr; 

    // Seed for the hash table
    seed          = seed; 
    stbds_rand_seed(seed);
    scene_id      = stbds_hash_string(filepath, seed); 
    bin_scene_id  = -1;

    // Friendly names for the material and mesh
    friendly_mat  = nullptr; 

    friendly_mesh = nullptr;  
    
    // Filenames for material and mesh
    filename_mat  = nullptr; 
    filename_mesh = nullptr; 
}

Scene::~Scene()
{
    arrfree(meshes);
    shfree(friendly_mat);
    shfree(friendly_mesh);
    shfree(filename_mat);
    shfree(filename_mesh);

    // MaterialLayout  *material_types  = nullptr; 
    meshes        = nullptr; 

    // Seed for the hash table
    seed          = -1; 
    scene_id      = -1; 
    bin_scene_id  = -1;

    // Friendly names for the material and mesh
    friendly_mat  = nullptr; 

    friendly_mesh = nullptr;  
    
    // Filenames for material and mesh
    filename_mat  = nullptr; 
    filename_mesh = nullptr; 
}

//-----------------------------------------------------------------------//
// Mesh related functions
//-----------------------------------------------------------------------//

size_t Scene::LoadMeshFromFile(EMeshFileType type, char *filename)
{
    switch(type)
    {
        case GLTF:
        {
            glTF::LoadMesh(filename);
        } break;
        case OBJ:
        case FBX:
        default:
        {
            printf("Model file type not supported!\n");
            return -1;
        } break;
    }
}

// Returns the index into the mesh array
size_t Scene::AddMesh(Mesh *mesh, char* filename, char *friendly)
{
    size_t mesh_id = arrlen(meshes);
    size_t mesh_file_id = (filename_mesh != nullptr) ? shlen(filename_mesh) : 0;
    size_t mesh_friend_id = (friendly_mesh != nullptr) ? shlen(friendly_mesh) : 0;

    // Insert into mesh list
    MeshElement me;
    me.filename_idx = mesh_file_id;
    me.friendly_idx = mesh_friend_id;
    me.mesh = mesh;
    arrput(meshes, me);

    // Insert into friendly hash list
    HashElement hefr;
    hefr.key = friendly;
    hefr.value = (void*)mesh_friend_id;
    shputs(friendly_mesh, hefr);

    // Insert into filename hash list
    HashElement hefi;
    hefi.key = filename;
    hefi.value = (void*)mesh_file_id;
    shputs(filename_mesh, hefi);

    return mesh_id;
}

Mesh *Scene::GetMeshByFriendlyName(char *friendly)
{
    HashElement he = shgets(friendly_mesh, friendly);
    return meshes[(size_t)he.value].mesh;
}

Mesh *Scene::GetMeshByFilename(char *filename)
{
    HashElement he = shgets(filename_mesh, filename);
    return meshes[(size_t)he.value].mesh;
}

u32 Scene::ChangeMeshFriendlyName(char *old_name, char *new_name)
{
    return 0;
}

u32 Scene::ChangeMeshFilename(char *old_name, char *new_name)
{
    return 0;
}