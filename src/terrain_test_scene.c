#include "terrain_test_scene.h"
#include <stdlib.h>
#include <string.h>
struct TgTerrainTestScene { TgTerrain terrain; TgGpuMeshVk *gpu_mesh; TgTerrainConfig config; TgTerrainHeightStats stats; };
int tg_terrain_test_scene_create(VkPhysicalDevice physical_device,VkDevice device,
 VkQueue graphics_queue,uint32_t queue_family_index,uint32_t seed,
 TgTerrainTestScene **out_scene,VkResult *out_vk_result){
 TgTerrainTestScene *scene; VkResult upload=VK_ERROR_INITIALIZATION_FAILED;
 if(out_vk_result)*out_vk_result=VK_ERROR_INITIALIZATION_FAILED;
 if(physical_device==VK_NULL_HANDLE||device==VK_NULL_HANDLE||graphics_queue==VK_NULL_HANDLE||!out_scene||*out_scene)return -1;
 scene=(TgTerrainTestScene*)calloc(1u,sizeof(*scene));
 if(!scene){if(out_vk_result)*out_vk_result=VK_ERROR_OUT_OF_HOST_MEMORY;return -1;}
 scene->config=tg_terrain_default_config(seed);
 if(tg_terrain_generate(&scene->config,&scene->terrain)!=0||
    tg_terrain_get_height_stats(&scene->terrain,&scene->stats)!=0)goto failed;
 if(tg_gpu_mesh_vk_upload(physical_device,device,graphics_queue,queue_family_index,
                          &scene->terrain.mesh,&scene->gpu_mesh,&upload)!=0)goto failed;
 if(out_vk_result)*out_vk_result=VK_SUCCESS;
 *out_scene=scene;return 0;
failed:
 if(out_vk_result)*out_vk_result=upload;
 tg_gpu_mesh_vk_destroy(device,scene->gpu_mesh);
 tg_terrain_destroy(&scene->terrain);free(scene);return -1;
}
void tg_terrain_test_scene_destroy(VkDevice device,TgTerrainTestScene *scene){
 if(!scene)return;tg_gpu_mesh_vk_destroy(device,scene->gpu_mesh);
 scene->gpu_mesh=NULL;tg_terrain_destroy(&scene->terrain);free(scene);
}
const TgGpuMeshVk *tg_terrain_test_scene_gpu_mesh(const TgTerrainTestScene *scene){return scene?scene->gpu_mesh:NULL;}
const TgTerrainConfig *tg_terrain_test_scene_config(const TgTerrainTestScene *scene){return scene?&scene->config:NULL;}
const TgTerrainHeightStats *tg_terrain_test_scene_stats(const TgTerrainTestScene *scene){return scene?&scene->stats:NULL;}
