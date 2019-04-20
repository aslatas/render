#include "SceneManager.h"


SceneManager::SceneManager()
{
    materials = nullptr;
    models = nullptr;
    sh_new_strdup(models);
    model_data = nullptr;
}
SceneManager::~SceneManager()
{
    if (models != nullptr)
    {
        shfree(models);
    }
}

ptrdiff_t SceneManager::LoadMaterial(const char* key)
{
    return (0);
}

ptrdiff_t SceneManager::LoadModel(const char* filename, ptrdiff_t mat_idx)
{
    // printf("Model being loaded: %s\n", filename);
    // Create a default model
    Temp* m = (Temp*)malloc(sizeof(Temp));
    m->material_index = mat_idx;

    // Place the model in the map
    shput(models, filename, m);

    // printf("Length of Model HashTable: %td\n", shlen(models));

    

    // HashModel hmo = shgets(models, &filename[0]);
    // printf("Another attempts to get the correct key: %td\n", hmo.value->material_index);

        // shdel(models, filename);


    // retrieve its index from the map
    return shgeti(models, filename);
}
ptrdiff_t SceneManager::GetModelIndex(const char* key)
{
    return shgeti(models, key);
}
HashModel SceneManager::GetModelStruct(const char* key)
{
    return shgets(models, key);
}

Model* SceneManager::GetModel(const char* key)
{
    return nullptr;
}


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

void SceneManager::PrintScene()
{
}

void SceneManager::PrintModelTable() 
{
    for (unsigned int i = 0; i < shlen(models); ++i) {
        printf("Index %d has the key %s\n", i, models[i].key);
    }
}
