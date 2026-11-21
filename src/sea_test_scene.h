#ifndef TORNIA_SEA_TEST_SCENE_H
#define TORNIA_SEA_TEST_SCENE_H

#include "gpu_mesh_vk.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TG_SEA_TEST_SCENE_WIDTH 40.0f
#define TG_SEA_TEST_SCENE_DEPTH 20.0f
#define TG_SEA_TEST_SCENE_COLUMNS 96u
#define TG_SEA_TEST_SCENE_ROWS 48u

typedef struct TgSeaTestScene TgSeaTestScene;

/*
 * Creates a subdivided bounded XZ sea grid and uploads its mesh.
 * The scene owns its CPU arrays and the opaque GPU mesh. The upload call
 * waits for completion before this function returns.
 *
 * The caller must externally synchronize graphics_queue. The queue family
 * must support graphics operations. out_scene must point to NULL.
 */
int tg_sea_test_scene_create(VkPhysicalDevice physical_device,
                             VkDevice device,
                             VkQueue graphics_queue,
                             uint32_t queue_family_index,
                             TgSeaTestScene **out_scene,
                             VkResult *out_vk_result);

/*
 * Destroys the GPU mesh while device is still valid, then releases CPU data.
 * The caller must ensure no submitted frame still references the mesh.
 */
void tg_sea_test_scene_destroy(VkDevice device, TgSeaTestScene *scene);

/* Borrowed GPU mesh; ownership remains with the scene. */
const TgGpuMeshVk *tg_sea_test_scene_gpu_mesh(const TgSeaTestScene *scene);

#ifdef __cplusplus
}
#endif

#endif
