#include "SceneManager.h"


SceneManager::SceneManager()
{
    materials = nullptr;
    models = nullptr;
    model_data = nullptr;
}
SceneManager::~SceneManager()
{
    if (models != nullptr)
    {
        shfree(models);
    }
}

int SceneManager::LoadMaterial(char* key)
{
}

int SceneManager::LoadModel(char* filename)
{
    printf("Model being loaded: %s\n", filename);
    // Create a default model
    Model* m = (Model*)malloc(sizeof(Model));

    // Place the model in the map
    shput(models, filename, m);

    printf("Length of Model HashTable: %d\n", shlen(models));

    for (int i = 0; i < shlen(models); ++i) {
        printf("Index %d has the key %s\n", i, models[i].key);
    }

    HashModel hmo = shgets(models, filename);
    printf("Another attempts to get the correct key: %d\n", shgeti(models, filename));

    // retrieve its index from the map
    return shgeti(models, filename);
}
ptrdiff_t SceneManager::GetModelIndex(char* key)
{
    return shgeti(models, key);
}
HashModel SceneManager::GetModelStruct(char* key)
{
    return shgets(models, key);
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
