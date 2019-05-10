
//------------------------------------------------//
// Test simple insert
//------------------------------------------------//
void test_add_one_mesh()
{
    Scene scene(102835, "no_file");

    
    char *test_file_a = "fileA";
    char *test_friendly_a = "friendlyA";
    Primitive attrib;
    attrib.vertex_count = 100;
    attrib.index_count = 100;
    attrib.uniform_index = 100;
    Mesh m;
    m.primitives = nullptr;
    arrput(m.primitives, attrib);
    
    scene.AddMesh(&m, test_file_a, test_friendly_a);

    // get the mesh by friendly name
    Mesh *mfr = scene.GetMeshByFriendlyName(test_friendly_a);
    assert(100 == mfr->primitives[0].vertex_count);
    assert(100 == mfr->primitives[0].index_count);
    assert(100 == mfr->primitives[0].uniform_index);
    // get mesh by filename
    mfr = scene.GetMeshByFilename(test_file_a);
    assert(100 == mfr->primitives[0].vertex_count);
    assert(100 == mfr->primitives[0].index_count);
    assert(100 == mfr->primitives[0].uniform_index);
}

void test_add_many_meshes()
{
    Scene scene(102835, "no_file");

    
    char *test_file_a = "fileA";
    char *test_friendly_a = "friendlyA";
    Primitive attrib;
    attrib.vertex_count = 100;
    attrib.index_count = 100;
    attrib.uniform_index = 100;
    Mesh m;
    m.primitives = nullptr;
    arrput(m.primitives, attrib);
    
    scene.AddMesh(&m, test_file_a, test_friendly_a);

    char *test_file_b = "fileB";
    char *test_friendly_b = "friendlyB";
    Primitive attrib2;
    attrib2.vertex_count = 200;
    attrib2.index_count = 200;
    attrib2.uniform_index = 200;
    Mesh m2;
    m2.primitives = nullptr;
    arrput(m2.primitives, attrib2);
    
    scene.AddMesh(&m2, test_file_b, test_friendly_b);

    char *test_file_c = "fileC";
    char *test_friendly_c = "friendlyC";
    Primitive attrib3;
    attrib3.vertex_count = 300;
    attrib3.index_count = 300;
    attrib3.uniform_index = 300;
    Mesh m3;
    m3.primitives = nullptr;
    arrput(m3.primitives, attrib3);
    
    scene.AddMesh(&m3, test_file_c, test_friendly_c);

    // Get mesh B
    // get the mesh by friendly name
    Mesh *mfr = scene.GetMeshByFriendlyName(test_friendly_b);
    assert(200 == mfr->primitives[0].vertex_count);
    assert(200 == mfr->primitives[0].index_count);
    assert(200 == mfr->primitives[0].uniform_index);
    // get mesh by filename
    mfr = scene.GetMeshByFilename(test_file_b);
    assert(200 == mfr->primitives[0].vertex_count);
    assert(200 == mfr->primitives[0].index_count);
    assert(200 == mfr->primitives[0].uniform_index);

    // Get mesh C
    // get the mesh by friendly name
    mfr = scene.GetMeshByFriendlyName(test_friendly_c);
    assert(300 == mfr->primitives[0].vertex_count);
    assert(300 == mfr->primitives[0].index_count);
    assert(300 == mfr->primitives[0].uniform_index);
    // get mesh by filename
    mfr = scene.GetMeshByFilename(test_file_c);
    assert(300 == mfr->primitives[0].vertex_count);
    assert(300 == mfr->primitives[0].index_count);
    assert(300 == mfr->primitives[0].uniform_index);

    // Get mesh A
    // get the mesh by friendly name
    mfr = scene.GetMeshByFriendlyName(test_friendly_a);
    assert(100 == mfr->primitives[0].vertex_count);
    assert(100 == mfr->primitives[0].index_count);
    assert(100 == mfr->primitives[0].uniform_index);
    // get mesh by filename
    mfr = scene.GetMeshByFilename(test_file_a);
    assert(100 == mfr->primitives[0].vertex_count);
    assert(100 == mfr->primitives[0].index_count);
    assert(100 == mfr->primitives[0].uniform_index);
}

void test_load_simple_mesh_from_file()
{
    Scene scene(102835, "no_file");
    scene.LoadMeshFromFile(Scene::GLTF, "../../../resources/models/Sponza/glTF/Sponza.gltf");
    scene.LoadMeshFromFile(Scene::GLTF, "../../../resources/models/Box/glTF/Box.gltf");
    scene.LoadMeshFromFile(Scene::GLTF, "this/is/a/filepath/test.none");
}