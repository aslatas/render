#include "GLTF.h"

using namespace glTF;

enum EGLTFExtension
{
    ASCII,
    GLB,
    INVALID
};

Mesh *glTF::LoadMesh(char *filename)
{
    EGLTFExtension extension;

    // Determine the file format and the friendly name
    char *friendly;
    char *format;
    char *cp = (char*)malloc(strlen(filename) + 1); // make a copy, just in case
    strcpy(cp, filename);

    friendly = strrchr(cp, '/') + 1; // the + 1 removes the special character
    format = strrchr(cp, '.') + 1;

         if (strncmp("glb", format, 3) == 0)  extension = GLB;
    else if (strncmp("gltf", format, 4) == 0) extension = ASCII;
    else                      extension = INVALID;

    free(cp);

    switch(extension)
    {
        case GLB:
        {
            printf("Found GLB file format!\n");
        } break;
        case ASCII:
        {
            printf("Found ASCII file format!\n");
        } break;
        case INVALID:
        default:
        {
            printf("Invalid file format for gltf files: %s\n", format);
            return nullptr;
        } break;
    }

    cgltf_options options = {(cgltf_file_type)0};
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, filename, &data);
    if (result == cgltf_result_success)
    {
	    printf("Successfully read file.\n");
	    cgltf_free(data);
    }
    else
    {
        printf("ERROR loading gltf file %s\n", filename);
        return nullptr;
    }

    return nullptr;
}