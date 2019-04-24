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
    Model * model = new Model;
    LoadGTLFModel(filename, *model, -1, mat_idx, -1);
    // Temp* m = (Temp*)malloc(sizeof(Temp));
    

    size_t hashed_key = stbds_hash_string(filename, models->seed);
    models->hash_table;

    HashModel hm;
    hm.key = hashed_key;
    hm.value = (void*)model;

    // Place the model in the map
    size_t idx = arrlen(models->hash_table);
    arrput(models->hash_table, hm);
    // retrieve its index from the map
    model->material_index = mat_idx;
    model->hash_index = idx;
    // TODO(Dustin): model data idx

    return idx;
}

// void SceneManager::AddModel(size_t model_idx, size_t mat_index)
// {
//     Model m = models->hash_table[model_idx];
//     SpatialModel m;
//     m.aabb = m->bounds;
//     m.material_index = mat_index;
//     m.hash_index = model_idx;

//     arrput(model_data, m);
// }

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
    printf("LOADING OCT TREE\n");
    printf("There are %d models in scene.\n", arrlen(models->hash_table));
    if (scene == nullptr)
    {
        printf("Scene Heirarchy not initialized! Please initialize before loading scene data into the heirarchy.\n");
        return;
    }

    // What probably should happen instead is we pass the reference of Model through to add.
    // The tree finds the Bin to add and the SpatialModel is allocated to that Bin.
    for (int i = 0; i < arrlen(models->hash_table); ++i)
    {
        Model *m = (Model*)models->hash_table[i].value;
        SpatialModel *sm = (SpatialModel*)malloc(sizeof(SpatialModel));
        memcpy(&sm->aabb, &m->bounds, sizeof(AABB_3D));
        sm->model_index = m->hash_index;
        sm->material_index = m->material_index;

        scene->Add(sm);
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
