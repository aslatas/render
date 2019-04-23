#include "SceneManager.h" 

/*
 * Things to note:
 *  HashTable is currently being faked - I do not have time to continue to play with the stbds_hashtable, I am using an array that contains the key,value pair. To retrieve a particular index, pass the key (string) to Get*Index(char*) and the index will be returned. Of course, this isn't as efficient as a HashTable, but this is a temporary fix to my current problem.  
 *
 * Assign Global Memory to HashTables ?
 * 
 * Make SceneManager a Singleton instance?
 */

//-------------------------------------------------------------------//
// SINGLETON
//-------------------------------------------------------------------//
SceneManager::SceneManager()
{
    materials = nullptr;

    models = (FakeHashTable*)malloc(sizeof(FakeHashTable));
    models->hash_table = nullptr;
    models->seed = 178433;
    // Seed the hash table with some number
    stbds_rand_seed(models->seed);

    model_data = nullptr;

    scene = nullptr;
}

// const SceneManager* SceneManager::GetInstance()                                              
// {
//     if (manager == nullptr)   
//         manager = new SceneManager();                                                                       
                                                                                 
//     return manager;                                                            
// }    

void SceneManager::Shutdown()
{
    if (models != nullptr) 
    {
        for (int i = 0; i < arrlen(models->hash_table); ++i)
        {
            free(models->hash_table[i].value);
        }
        arrfree(models->hash_table);
        free(models);
    }

    if (model_data != nullptr)
    {
        arrfree(model_data);
    }
}

SceneManager::~SceneManager() = default;

size_t SceneManager::LoadMaterial(char* key)
{
    return (0);
}

size_t SceneManager::LoadModel(char* filename, u32 mat_idx)
{
    // Create a default model
    Temp* m = (Temp*)malloc(sizeof(Temp));
    m->material_index = mat_idx;

    size_t hashed_key = stbds_hash_string(filename, models->seed);
    models->hash_table;

    HashModel hm;
    hm.key = hashed_key;
    hm.value = (void*)m;

    // Place the model in the map
    size_t idx = arrlen(models->hash_table);
    arrput(models->hash_table, hm);
    // retrieve its index from the map
    return idx;
}

void SceneManager::AddModel(float* min, float* max, size_t mat_index, int val)
{
    Model m;
    m.aabb = *Create3DAxisAlignedBoundingBox(min, max);
    m.val = val;
    m.material_index = mat_index;
    m.hash_index = -1;

    arrput(model_data, m);
}

size_t SceneManager::GetModelIndex(char* key)
{
    size_t hashed_key = stbds_hash_string(key, models->seed);
    for (u32 i = 0; i < arrlen(models->hash_table); ++i) {
        if (hashed_key == models->hash_table[i].key)
        {
            return i;
        }
    }
    //return shgets(models, key);
    return -1;
}
HashModel* SceneManager::GetModelStruct(char* key)
{
    size_t hashed_key = stbds_hash_string(key, models->seed);
    for (int i = 0; i < arrlen(models->hash_table); ++i) {
        if (hashed_key == models->hash_table[i].key)
        {
            return &models->hash_table[i];
        }
    }
    //return shgets(models, key);
    return nullptr;
}

//Model* SceneManager::GetModel(const char* key)
//{
//    return nullptr;
//}

// Creates a spatial heirarchy for the scene
void SceneManager::CreateSpatialHeirarchy(float *min, float *max)
{
    scene = new OctTree(min, max);
}

void SceneManager::LoadOctTree()
{
    if (scene == nullptr)
    {
        printf("Scene Heirarchy not initialized! Please initialize before loading scene data into the heirarchy.\n");
        return;
    }

    for (int i = 0; i < arrlen(model_data); ++i)
    {
        scene->Add(&model_data[i]);
    }
}

void SceneManager::FrustumCullOctTree()
{
}
void SceneManager::OcclusionCullOctTree()
{
}
void SceneManager::GetVisibleData()
{
}

void SceneManager::PrintScene()
{
    size_t len = arrlen(model_data);
    printf("There are %zu objects in the scene", arrlen(model_data));
    scene->Print();
}

void SceneManager::PrintModelTable() 
{
    //for (unsigned int i = 0; i < shlen(models); ++i) {
    //    printf("Index %d has the key %s\n", i, models[i].key);
    //}
}
