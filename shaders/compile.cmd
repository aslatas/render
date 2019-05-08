@echo off

if not exist spv mkdir spv

pushd spv

glslc ..\engine_default.vert -o engine_default_vert.spv
glslc ..\engine_default.frag -o engine_default_frag.spv

glslc ..\shader.vert -o vert.spv
glslc ..\shader.frag -o frag.spv

glslc ..\shader2.vert -o vert2.spv
glslc ..\shader2.frag -o frag2.spv

glslc ..\stencil.vert -o stencil_vert.spv

glslc ..\outline.vert -o outline_vert.spv
glslc ..\outline.frag -o outline_frag.spv

glslc ..\text.vert -o text_vert.spv
glslc ..\text.frag -o text_frag.spv

glslc ..\fill_vcolor.vert -o fill_vcolor_vert.spv
glslc ..\fill_vcolor.frag -o fill_vcolor_frag.spv

glslc ..\bounds_box.vert -o bounds_box_vert.spv
glslc ..\bounds_box.frag -o bounds_box_frag.spv

popd