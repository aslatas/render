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
    


    material_types = nullptr;

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
    for (u32 i = 0; i < arrlen(material_types); ++i) {
        for (u32 j = 0; j < arrlen(material_types[i].materials); ++j) {
            for (u32 k = 0; k < arrlen(material_types[i].materials[j].models); ++k) {
                DestroyModelSeparateDataTest(&material_types[i].materials[j].models[k]);
            }
            arrfree(material_types[i].materials[j].models);
        }
    }
    // Destroy materials.
    for (u32 i = 0; i < arrlen(material_types); ++i) {
        for (u32 j = 0; j < arrlen(material_types[i].materials); ++j) {
            DestroyPipeline(material_types[i].materials[j].pipeline);
        }
        
        DestroyPipelineLayout(material_types[i].pipeline_layout);
        arrfree(material_types[i].materials);
    }
    arrfree(material_types);

    // if (models != nullptr) 
    // {
    //     for (int i = 0; i < arrlen(models->hash_table); ++i)
    //     {
    //         free(models->hash_table[i].value);
    //     }
    //     arrfree(models->hash_table);
    //     free(models);
    // }

    // if (model_data != nullptr)
    // {
    //     arrfree(model_data);
    // }
}

SceneManager::~SceneManager() = default;

u32 SceneManager::AddMaterialType(MaterialLayout *layout)
{
    arrput(material_types, *layout);
    return (u32)arrlen(material_types) - 1;
}

u32 SceneManager::AddMaterial(Material* mat, u32 layout_idx)
{
    // MaterialLayout* layout = material_types[layout_idx];
    arrput(material_types[layout_idx].materials, *mat);
    return (u32)arrlen(material_types[layout_idx].materials) - 1;
}

// size_t SceneManager::LoadModel(char* filename, u32 mat_idx)
// {
//     // Create a default model
//     Model * model = new Model;
//     LoadGTLFModel(filename, *model, -1, mat_idx, -1);
//     // Temp* m = (Temp*)malloc(sizeof(Temp));
    

//     size_t hashed_key = stbds_hash_string(filename, models->seed);
//     models->hash_table;

//     HashModel hm;
//     hm.key = hashed_key;
//     hm.value = (void*)model;

//     // Place the model in the map
//     size_t idx = arrlen(models->hash_table);
//     arrput(models->hash_table, hm);
//     // retrieve its index from the map
//     model->material_index = mat_idx;
//     model->hash_index = idx;
//     // TODO(Dustin): model data idx

//     return idx;
// }

// anything else?
void SceneManager::AddModel(char* key, Model* model)
{
    HashModel hm;
    hm.key = stbds_hash_string(key, models->seed);
    hm.value = (void*)model;
    // arrput(models->hash_table, hm);
    arrput(model_data, *model);
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

// eww, a getter
MaterialLayout* SceneManager::GetMaterialLayout(u32 mat_layout_idx)
{
    return &material_types[mat_layout_idx];
}

Material* SceneManager::GetMaterial(u32 mat_layout_idx, u32 mat_idx)
{
    return &material_types[mat_layout_idx].materials[mat_idx];
}

u32 SceneManager::GetNumberOfModelsInScene()
{
    // size_t l = arrlen(models->hash_table);
    // return (u32)arrlen(models->hash_table);
    return (u32)arrlen(model_data);
}

Model* SceneManager::GetModelByIndex(u32 mod_idx)
{
    // return (Model*)models->hash_table[mod_idx].value;
    return &model_data[mod_idx];
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
    printf("There are %td models in scene.\n", arrlen(models->hash_table));
    if (scene == nullptr)
    {
        printf("Scene Heirarchy not initialized! Please initialize before loading scene data into the heirarchy.\n");
        return;
    }

    // What probably should happen instead is we pass the reference of Model through to add.
    // The tree finds the Bin to add and the SpatialModel is allocated to that Bin.
    // for (int i = 0; i < arrlen(models->hash_table); ++i)
    for (int i = 0; i < arrlen(model_data); ++i)
    {
        // Model *m = (Model*)models->hash_table[i].value;
        Model *m = &model_data[i];
        SpatialModel *sm = (SpatialModel*)malloc(sizeof(SpatialModel));
        memcpy(&sm->aabb, &m->bounds, sizeof(AABB_3D));
        sm->model_index = i;
        sm->material_type_idx = m->material_type; // index into material type
        sm->material_idx = m->shader_id; // index into material
        sm->isVisible = true;

        scene->Add(sm);
    }
}

void SceneManager::FrustumCull(Camera::Frustum *frustum)
{
    scene->UpdateFrustumVisibility(frustum);
}
void SceneManager::OcclusionCullOctTree()
{
}

RenderSceneMaterial* SceneManager::GetVisibleData()
{
    // list of visible spatial models
    SpatialModel *sm = scene->GetAllVisibleData();

    /*
u32 model_index;
    u32 material_type_idx; // index into material type list
    u32 material_idx;     // index into material list
    */
    RenderSceneMaterial* rsml = nullptr;
    for (int i = 0; i < arrlen(sm); ++i)
    {
        // check if the material type exists
        bool type_exist = false;
        for (int j = 0; j < arrlen(rsml); ++j)
        {
            if (rsml[j].mat_layout_idx == sm[i].material_type_idx)
            {
                type_exist = true;

                // check if material exists
                bool mat_exist = false;
                for (int k = 0; k < arrlen(rsml[j].scene_materials); ++k)
                {
                    // add model
                    if (rsml[j].scene_materials[k].mat_idx == sm[i].material_idx)
                    {
                        mat_exist = true;
                        size_t mat_type = sm[i].material_type_idx;
                        size_t mat = sm[i].material_idx;
                        arrput(rsml[j].scene_materials[k].model_idx, sm[i].model_index);
                        break;
                    }
                }

                if (!mat_exist)
                {
                    // add mat and model
                    scene_mat mat;
                    mat.mat_idx = sm[i].material_idx;
                    mat.model_idx = nullptr;
                    
                    size_t idx_mat = arrlen(rsml[j].scene_materials);
                    arrput(rsml[j].scene_materials, mat);
                    arrput(rsml[j].scene_materials[idx_mat].model_idx, sm[i].model_index);
                }
                break;
            }
        }

        if (!type_exist)
        {
            // add layout, mat, and model
            size_t idx = arrlen(rsml);
            RenderSceneMaterial rsm;
            rsm.mat_layout_idx = sm[i].material_type_idx;
            rsm.scene_materials = nullptr;
            scene_mat mat;
            mat.mat_idx = sm[i].material_idx;
            mat.model_idx = nullptr;
            arrput(rsml, rsm);

            size_t idx_mat = arrlen(rsml[idx].scene_materials);
            arrput(rsml[idx].scene_materials, mat);
            arrput(rsml[idx].scene_materials[idx_mat].model_idx, sm[i].model_index);
        }
    }

    arrfree(sm);
    return rsml;
}

void SceneManager::PrintScene()
{
    printf("PRINTG SCENE:\n\n");
    printf("There are %td models in the scene.\n\n", arrlen(model_data));
    scene->Print();
}

void SceneManager::PrintModelTable() 
{
    //for (unsigned int i = 0; i < shlen(models); ++i) {
    //    printf("Index %d has the key %s\n", i, models[i].key);
    //}
}
