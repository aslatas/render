@echo off
C:\VulkanSDK\1.1.97.0\Bin32\glslangValidator.exe -V shader.vert -o vert.spv
C:\VulkanSDK\1.1.97.0\Bin32\glslangValidator.exe -V shader.frag -o frag.spv

C:\VulkanSDK\1.1.97.0\Bin32\glslangValidator.exe -V shader2.vert -o vert2.spv
C:\VulkanSDK\1.1.97.0\Bin32\glslangValidator.exe -V shader2.frag -o frag2.spv

C:\VulkanSDK\1.1.97.0\Bin32\glslangValidator.exe -V stencil.vert -o stencil_vert.spv

C:\VulkanSDK\1.1.97.0\Bin32\glslangValidator.exe -V outline.vert -o outline_vert.spv
C:\VulkanSDK\1.1.97.0\Bin32\glslangValidator.exe -V outline.frag -o outline_frag.spv