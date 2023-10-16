glslc PBR.Geometry.Deferred.vert -o build/PBR.Geometry.Deferred.Vert.spv
glslc PBR.Geometry.Deferred.frag -o build/PBR.Geometry.Deferred.Frag.spv

glslc PBR.Lighting.Deferred.vert -o build/PBR.Lighting.Deferred.Vert.spv
glslc PBR.Lighting.Deferred.frag -o build/PBR.Lighting.Deferred.Frag.spv

glslc PBR.Forward.vert -o build/PBR.Forward.Vert.spv
glslc PBR.Forward.frag -o build/PBR.Forward.Frag.spv

glslc Skybox.vert -o build/Skybox.Vert.spv
glslc Skybox.frag -o build/Skybox.Frag.spv

glslc Blending.vert -o build/Blending.Vert.spv
glslc Blending.frag -o build/Blending.Frag.spv

glslc ShadowMap.vert -o build/ShadowMap.Vert.spv
glslc ShadowMap.frag -o build/ShadowMap.Frag.spv

glslc post_processing/ToneMapping.vert -o build/ToneMapping.Vert.spv
glslc post_processing/ToneMapping.frag -o build/ToneMapping.Frag.spv

glslc -fshader-stage=compute post_processing/Bloom.Compute.glsl -o build/Bloom.Compute.spv
glslc -fshader-stage=compute EquirectangularToCubemap.Compute.glsl -o build/EquirectangularToCubemap.Compute.spv
