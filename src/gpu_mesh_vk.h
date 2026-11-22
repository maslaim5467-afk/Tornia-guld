#ifndef TORNIA_GPU_MESH_VK_H
#define TORNIA_GPU_MESH_VK_H

#include "mesh3d.h"

#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TgGpuMeshVk TgGpuMeshVk;

/*
 * Uploads an immutable CPU mesh into a backend-owned combined vertex/index
 * VkBuffer. The function creates a temporary command pool and fence, submits
 * the upload on graphics_queue, and waits for completion before returning.
 *
 * The caller must externally synchronize access to graphics_queue and must
 * pass a queue from queue_family_index with graphics capability.
 *
 * out_mesh must point to NULL. On failure it remains NULL and, when supplied,
 * out_vk_result receives the Vulkan error (or VK_ERROR_INITIALIZATION_FAILED
 * for invalid CPU-side input).
 *
 * Returns 0 on success and -1 on failure.
 */
int tg_gpu_mesh_vk_upload(VkPhysicalDevice physical_device,
                          VkDevice device,
                          VkQueue graphics_queue,
                          uint32_t queue_family_index,
                          const TgMesh3D *source,
                          TgGpuMeshVk **out_mesh,
                          VkResult *out_vk_result);

/*
 * Destroys a mesh uploaded for device. The caller must ensure no submitted
 * command buffer can still reference the mesh. device must still be valid.
 */
void tg_gpu_mesh_vk_destroy(VkDevice device, TgGpuMeshVk *mesh);

/* Accessors return borrowed draw metadata; they do not transfer ownership. */
VkBuffer tg_gpu_mesh_vk_buffer(const TgGpuMeshVk *mesh);
VkDeviceSize tg_gpu_mesh_vk_vertex_offset(const TgGpuMeshVk *mesh);
VkDeviceSize tg_gpu_mesh_vk_index_offset(const TgGpuMeshVk *mesh);
uint32_t tg_gpu_mesh_vk_index_count(const TgGpuMeshVk *mesh);
VkBool32 tg_gpu_mesh_vk_used_staging(const TgGpuMeshVk *mesh);
VkMemoryPropertyFlags tg_gpu_mesh_vk_memory_properties(
    const TgGpuMeshVk *mesh);

#ifdef __cplusplus
}
#endif

#endif
