#ifndef GLTF_H

/*
TODO still:
- handle having multiple primitve - each with its own model matrix
- pos, rot, scl
- min/max per primitive
- textures and materials
*/

namespace glTF
{
    Mesh *LoadMesh(char *filename);
}; 

#define GLTF_H
#endif