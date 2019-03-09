@echo off
C:\VulkanSDK\1.1.97.0\Bin32\glslangValidator.exe -V -s shader.vert -o vert.spv
C:\VulkanSDK\1.1.97.0\Bin32\glslangValidator.exe -V -s shader.frag -o frag.spv

C:\VulkanSDK\1.1.97.0\Bin32\glslangValidator.exe -V -s shader2.vert -o vert2.spv
C:\VulkanSDK\1.1.97.0\Bin32\glslangValidator.exe -V -s shader2.frag -o frag2.spv

C:\VulkanSDK\1.1.97.0\Bin32\glslangValidator.exe -V -s stencil.vert -o stencil_vert.spv

C:\VulkanSDK\1.1.97.0\Bin32\glslangValidator.exe -V -s outline.vert -o outline_vert.spv
C:\VulkanSDK\1.1.97.0\Bin32\glslangValidator.exe -V -s outline.frag -o outline_frag.spv

C:\VulkanSDK\1.1.97.0\Bin32\glslangValidator.exe -V -s text.vert -o text_vert.spv
C:\VulkanSDK\1.1.97.0\Bin32\glslangValidator.exe -V -s text.frag -o text_frag.spv


C:\VulkanSDK\1.1.97.0\Bin32\glslangValidator.exe -V fill_vcolor.vert -o fill_vcolor_vert.spv
C:\VulkanSDK\1.1.97.0\Bin32\glslangValidator.exe -V fill_vcolor.frag -o fill_vcolor_frag.spv