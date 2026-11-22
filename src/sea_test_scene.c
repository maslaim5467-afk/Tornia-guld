#include "sea_test_scene.h"

#include "mesh3d.h"

#include <stdlib.h>

struct TgSeaTestScene {
    TgMeshVertex3D *vertices;
    uint32_t *indices;
    TgMesh3D cpu_mesh;
    TgGpuMeshVk *gpu_mesh;
};

static int build_sea_grid(TgSeaTestScene *scene) {
    const size_t columns = (size_t)TG_SEA_TEST_SCENE_COLUMNS + 1u;
    const size_t rows = (size_t)TG_SEA_TEST_SCENE_ROWS + 1u;
    const size_t vertex_count = columns * rows;
    const size_t index_count = (size_t)TG_SEA_TEST_SCENE_COLUMNS * TG_SEA_TEST_SCENE_ROWS * 6u;
    size_t z, x, cursor = 0u;

    scene->vertices = (TgMeshVertex3D *)calloc(vertex_count, sizeof(*scene->vertices));
    scene->indices = (uint32_t *)calloc(index_count, sizeof(*scene->indices));
    if (scene->vertices == NULL || scene->indices == NULL) return -1;

    for (z = 0u; z < rows; ++z) {
        for (x = 0u; x < columns; ++x) {
            const float u = (float)x / (float)TG_SEA_TEST_SCENE_COLUMNS;
            const float v = (float)z / (float)TG_SEA_TEST_SCENE_ROWS;
            const size_t index = z * columns + x;
            scene->vertices[index].position.x = (u - 0.5f) * TG_SEA_TEST_SCENE_WIDTH;
            scene->vertices[index].position.y = 0.0f;
            scene->vertices[index].position.z = (v - 0.5f) * TG_SEA_TEST_SCENE_DEPTH;
        }
    }

    for (z = 0u; z < (size_t)TG_SEA_TEST_SCENE_ROWS; ++z) {
        for (x = 0u; x < (size_t)TG_SEA_TEST_SCENE_COLUMNS; ++x) {
            const uint32_t a = (uint32_t)(z * columns + x);
            const uint32_t b = a + 1u;
            const uint32_t d = (uint32_t)((z + 1u) * columns + x);
            const uint32_t c = d + 1u;
            scene->indices[cursor++] = a;
            scene->indices[cursor++] = c;
            scene->indices[cursor++] = b;
            scene->indices[cursor++] = a;
            scene->indices[cursor++] = d;
            scene->indices[cursor++] = c;
        }
    }

    return 0;
}

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

    if (build_sea_grid(scene) != 0 ||
        tg_mesh3d_init(&scene->cpu_mesh,
                       scene->vertices,
                       ((size_t)TG_SEA_TEST_SCENE_COLUMNS + 1u) * ((size_t)TG_SEA_TEST_SCENE_ROWS + 1u),
                       scene->indices,
                       (size_t)TG_SEA_TEST_SCENE_COLUMNS * (size_t)TG_SEA_TEST_SCENE_ROWS * 6u) != 0) {
        free(scene->vertices);
        free(scene->indices);
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
        free(scene->vertices);
        free(scene->indices);
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
    free(scene->vertices);
    free(scene->indices);
    scene->vertices = NULL;
    scene->indices = NULL;
    free(scene);
}

const TgGpuMeshVk *tg_sea_test_scene_gpu_mesh(const TgSeaTestScene *scene) {
    return scene != NULL ? scene->gpu_mesh : NULL;
}
