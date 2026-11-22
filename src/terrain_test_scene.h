#ifndef TORNIA_TERRAIN_TEST_SCENE_H
#define TORNIA_TERRAIN_TEST_SCENE_H
#include "gpu_mesh_vk.h"
#include "terrain_generator.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct TgTerrainTestScene TgTerrainTestScene;
/* Builds the approved 512 m rolling heightfield and uploads it once.
 * CPU generation and upload are synchronous; call during scene loading,
 * with the graphics queue externally synchronized. */
int tg_terrain_test_scene_create(VkPhysicalDevice physical_device,VkDevice device,
 VkQueue graphics_queue,uint32_t queue_family_index,uint32_t seed,
 TgTerrainTestScene **out_scene,VkResult *out_vk_result);
void tg_terrain_test_scene_destroy(VkDevice device,TgTerrainTestScene *scene);
const TgGpuMeshVk *tg_terrain_test_scene_gpu_mesh(const TgTerrainTestScene *scene);
const TgTerrainConfig *tg_terrain_test_scene_config(const TgTerrainTestScene *scene);
const TgTerrainHeightStats *tg_terrain_test_scene_stats(const TgTerrainTestScene *scene);
#ifdef __cplusplus
}
#endif
#endif
