#include "SceneManager.h"

/*
 * Things to note:
 *  HashTable is currently being faked - I do not have time to continue to play with the stbds_hashtable, I am using an array that contains the key,value pair. To retrieve a particular index, pass the key (string) to Get*Index(char*) and the index will be returned. Of course, this isn't as efficient as a HashTable, but this is a temporary fix to my current problem.  
 *
 * Assign Global Memory to HashTables ?
 *
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
}

const SceneManager* SceneManager::GetInstance()                                              
{
    if (manager == nullptr)   
        manager = new SceneManager();                                                                       
                                                                                 
    return manager;                                                            
}    

void SceneManager::Shutdown() const
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
}

SceneManager::~SceneManager() = default;

size_t SceneManager::LoadMaterial(char* key) const
{
    return (0);
}

size_t SceneManager::LoadModel(char* filename, ptrdiff_t mat_idx) const
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
size_t SceneManager::GetModelIndex(char* key) const
{
    size_t hashed_key = stbds_hash_string(key, models->seed);
    for (size_t i = 0; i < arrlen(models->hash_table); ++i) {
        if (hashed_key == models->hash_table[i].key)
        {
            return i;
        }
    }
    //return shgets(models, key);
    return -1;
}
HashModel* SceneManager::GetModelStruct(char* key) const
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


void SceneManager::LoadOctTree()
{
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

void SceneManager::PrintScene() const
{
}

void SceneManager::PrintModelTable() 
{
    //for (unsigned int i = 0; i < shlen(models); ++i) {
    //    printf("Index %d has the key %s\n", i, models[i].key);
    //}
}
