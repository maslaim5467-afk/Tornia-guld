#include "sea_test_scene.h"

#include "mesh3d.h"

#include <stdlib.h>

struct TgSeaTestScene {
    TgMeshVertex3D vertices[4];
    uint32_t indices[6];
    TgMesh3D cpu_mesh;
    TgGpuMeshVk *gpu_mesh;
};

int tg_sea_test_scene_create(VkPhysicalDevice physical_device,
                             VkDevice device,
                             VkQueue graphics_queue,
                             uint32_t queue_family_index,
                             TgSeaTestScene **out_scene,
                             VkResult *out_vk_result) {
    TgSeaTestScene *scene;
    VkResult upload_result = VK_ERROR_INITIALIZATION_FAILED;

    if (out_vk_result != NULL) {
        *out_vk_result = VK_ERROR_INITIALIZATION_FAILED;
    }

    if (physical_device == VK_NULL_HANDLE ||
        device == VK_NULL_HANDLE ||
        graphics_queue == VK_NULL_HANDLE ||
        out_scene == NULL ||
        *out_scene != NULL) {
        return -1;
    }

    scene = (TgSeaTestScene *)calloc(1u, sizeof(*scene));
    if (scene == NULL) {
        if (out_vk_result != NULL) {
            *out_vk_result = VK_ERROR_OUT_OF_HOST_MEMORY;
        }
        return -1;
    }

    if (tg_mesh3d_write_plane_xz(TG_SEA_TEST_SCENE_WIDTH,
                                 TG_SEA_TEST_SCENE_DEPTH,
                                 scene->vertices,
                                 scene->indices) != 0 ||
        tg_mesh3d_init(&scene->cpu_mesh,
                       scene->vertices,
                       4u,
                       scene->indices,
                       6u) != 0) {
        free(scene);
        return -1;
    }

    if (tg_gpu_mesh_vk_upload(physical_device,
                             device,
                             graphics_queue,
                             queue_family_index,
                             &scene->cpu_mesh,
                             &scene->gpu_mesh,
                             &upload_result) != 0) {
        free(scene);
        if (out_vk_result != NULL) {
            *out_vk_result = upload_result;
        }
        return -1;
    }

    if (out_vk_result != NULL) {
        *out_vk_result = VK_SUCCESS;
    }
    *out_scene = scene;
    return 0;
}

void tg_sea_test_scene_destroy(VkDevice device, TgSeaTestScene *scene) {
    if (scene == NULL) return;

    tg_gpu_mesh_vk_destroy(device, scene->gpu_mesh);
    scene->gpu_mesh = NULL;
    free(scene);
}

const TgGpuMeshVk *tg_sea_test_scene_gpu_mesh(const TgSeaTestScene *scene) {
    return scene != NULL ? scene->gpu_mesh : NULL;
}
