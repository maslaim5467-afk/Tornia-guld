#include "gpu_mesh_vk.h"

#include "gpu_mesh_layout.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct TgGpuMeshVk {
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDeviceSize vertex_offset;
    VkDeviceSize index_offset;
    VkDeviceSize allocation_size;
    uint32_t index_count;
    VkMemoryPropertyFlags memory_properties;
    VkBool32 used_staging;
};

typedef struct {
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDeviceSize allocation_size;
    VkMemoryPropertyFlags memory_properties;
} TgVkBufferAllocation;

static void set_vk_result(VkResult *out_vk_result, VkResult value) {
    if (out_vk_result != NULL) {
        *out_vk_result = value;
    }
}

static void destroy_buffer_allocation(VkDevice device,
                                      TgVkBufferAllocation *allocation) {
    if (allocation == NULL) return;

    if (allocation->buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, allocation->buffer, NULL);
    }
    if (allocation->memory != VK_NULL_HANDLE) {
        vkFreeMemory(device, allocation->memory, NULL);
    }

    memset(allocation, 0, sizeof(*allocation));
}

static int find_memory_type(const VkPhysicalDeviceMemoryProperties *properties,
                            uint32_t type_bits,
                            VkMemoryPropertyFlags required,
                            VkMemoryPropertyFlags preferred,
                            uint32_t *out_index,
                            VkMemoryPropertyFlags *out_flags) {
    uint32_t i;
    int fallback = -1;

    if (properties == NULL || out_index == NULL || out_flags == NULL) {
        return -1;
    }

    for (i = 0; i < properties->memoryTypeCount; ++i) {
        VkMemoryPropertyFlags flags;

        if ((type_bits & (1u << i)) == 0u) continue;
        flags = properties->memoryTypes[i].propertyFlags;
        if ((flags & required) != required) continue;

        if ((flags & preferred) == preferred) {
            *out_index = i;
            *out_flags = flags;
            return 0;
        }

        if (fallback < 0) fallback = (int)i;
    }

    if (fallback < 0) return -1;

    *out_index = (uint32_t)fallback;
    *out_flags = properties->memoryTypes[fallback].propertyFlags;
    return 0;
}

static VkResult allocate_and_bind(VkDevice device,
                                  VkBuffer buffer,
                                  const VkMemoryRequirements *requirements,
                                  uint32_t memory_type_index,
                                  TgVkBufferAllocation *out_allocation) {
    VkMemoryAllocateInfo allocation_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = requirements->size,
        .memoryTypeIndex = memory_type_index
    };
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkResult result;

    result = vkAllocateMemory(device, &allocation_info, NULL, &memory);
    if (result != VK_SUCCESS) return result;

    result = vkBindBufferMemory(device, buffer, memory, 0u);
    if (result != VK_SUCCESS) {
        vkFreeMemory(device, memory, NULL);
        return result;
    }

    out_allocation->buffer = buffer;
    out_allocation->memory = memory;
    out_allocation->allocation_size = requirements->size;
    return VK_SUCCESS;
}

static VkResult create_destination_buffer(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkDeviceSize size,
    TgVkBufferAllocation *out_allocation) {
    VkBufferCreateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                 VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    VkPhysicalDeviceMemoryProperties properties;
    VkMemoryRequirements requirements;
    VkMemoryPropertyFlags selected_flags = 0u;
    uint32_t memory_type_index = 0u;
    VkResult result;

    result = vkCreateBuffer(device, &buffer_info, NULL,
                            &out_allocation->buffer);
    if (result != VK_SUCCESS) return result;

    vkGetBufferMemoryRequirements(device, out_allocation->buffer,
                                  &requirements);
    vkGetPhysicalDeviceMemoryProperties(physical_device, &properties);

    if (find_memory_type(&properties, requirements.memoryTypeBits,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0u,
                         &memory_type_index, &selected_flags) != 0) {
        if (find_memory_type(&properties, requirements.memoryTypeBits,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             &memory_type_index, &selected_flags) != 0) {
            vkDestroyBuffer(device, out_allocation->buffer, NULL);
            memset(out_allocation, 0, sizeof(*out_allocation));
            return VK_ERROR_FEATURE_NOT_PRESENT;
        }
    }

    result = allocate_and_bind(device, out_allocation->buffer, &requirements,
                               memory_type_index, out_allocation);
    if (result != VK_SUCCESS) {
        vkDestroyBuffer(device, out_allocation->buffer, NULL);
        memset(out_allocation, 0, sizeof(*out_allocation));
        return result;
    }

    out_allocation->memory_properties = selected_flags;
    return VK_SUCCESS;
}

static VkResult create_staging_buffer(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkDeviceSize size,
    TgVkBufferAllocation *out_allocation) {
    VkBufferCreateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    VkPhysicalDeviceMemoryProperties properties;
    VkMemoryRequirements requirements;
    VkMemoryPropertyFlags selected_flags = 0u;
    uint32_t memory_type_index = 0u;
    VkResult result;

    result = vkCreateBuffer(device, &buffer_info, NULL,
                            &out_allocation->buffer);
    if (result != VK_SUCCESS) return result;

    vkGetBufferMemoryRequirements(device, out_allocation->buffer,
                                  &requirements);
    vkGetPhysicalDeviceMemoryProperties(physical_device, &properties);
    if (find_memory_type(&properties, requirements.memoryTypeBits,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         &memory_type_index, &selected_flags) != 0) {
        vkDestroyBuffer(device, out_allocation->buffer, NULL);
        memset(out_allocation, 0, sizeof(*out_allocation));
        return VK_ERROR_FEATURE_NOT_PRESENT;
    }

    result = allocate_and_bind(device, out_allocation->buffer, &requirements,
                               memory_type_index, out_allocation);
    if (result != VK_SUCCESS) {
        vkDestroyBuffer(device, out_allocation->buffer, NULL);
        memset(out_allocation, 0, sizeof(*out_allocation));
        return result;
    }

    out_allocation->memory_properties = selected_flags;
    return VK_SUCCESS;
}

static VkResult map_and_copy(VkDevice device,
                             VkDeviceMemory memory,
                             VkDeviceSize allocation_size,
                             VkMemoryPropertyFlags memory_properties,
                             VkDeviceSize vertex_bytes,
                             VkDeviceSize index_offset,
                             VkDeviceSize index_bytes,
                             const TgMesh3D *source) {
    void *mapped = NULL;
    VkResult result;

    result = vkMapMemory(device, memory, 0u, allocation_size, 0u, &mapped);
    if (result != VK_SUCCESS) return result;

    memcpy(mapped, source->vertices, (size_t)vertex_bytes);
    memcpy((unsigned char *)mapped + (size_t)index_offset,
           source->indices, (size_t)index_bytes);

    if ((memory_properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0u) {
        VkMappedMemoryRange range = {
            .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
            .memory = memory,
            .offset = 0u,
            .size = VK_WHOLE_SIZE
        };
        result = vkFlushMappedMemoryRanges(device, 1u, &range);
        vkUnmapMemory(device, memory);
        return result;
    }

    vkUnmapMemory(device, memory);
    return VK_SUCCESS;
}

static VkResult submit_upload(VkDevice device,
                              VkQueue graphics_queue,
                              uint32_t queue_family_index,
                              const TgVkBufferAllocation *destination,
                              const TgVkBufferAllocation *staging,
                              const TgGpuMeshLayout *layout,
                              VkBool32 use_staging) {
    VkCommandPool command_pool = VK_NULL_HANDLE;
    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    VkFence fence = VK_NULL_HANDLE;
    VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = queue_family_index
    };
    VkCommandBufferAllocateInfo command_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1u
    };
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
    };
    VkResult result;
    VkBool32 submitted = VK_FALSE;

    result = vkCreateCommandPool(device, &pool_info, NULL, &command_pool);
    if (result != VK_SUCCESS) goto cleanup;

    command_info.commandPool = command_pool;
    result = vkAllocateCommandBuffers(device, &command_info, &command_buffer);
    if (result != VK_SUCCESS) goto cleanup;

    result = vkBeginCommandBuffer(command_buffer, &begin_info);
    if (result != VK_SUCCESS) goto cleanup;

    if (use_staging) {
        VkBufferCopy copies[2] = {
            {
                .srcOffset = 0u,
                .dstOffset = layout->vertex_offset,
                .size = layout->vertex_bytes
            },
            {
                .srcOffset = layout->index_offset,
                .dstOffset = layout->index_offset,
                .size = layout->index_bytes
            }
        };
        VkBufferMemoryBarrier barrier = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
                             VK_ACCESS_INDEX_READ_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = destination->buffer,
            .offset = 0u,
            .size = layout->total_bytes
        };

        vkCmdCopyBuffer(command_buffer, staging->buffer,
                        destination->buffer, 2u, copies);
        vkCmdPipelineBarrier(command_buffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                             0u, 0u, NULL, 1u, &barrier, 0u, NULL);
    } else {
        VkBufferMemoryBarrier barrier = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_HOST_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
                             VK_ACCESS_INDEX_READ_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = destination->buffer,
            .offset = 0u,
            .size = layout->total_bytes
        };

        vkCmdPipelineBarrier(command_buffer,
                             VK_PIPELINE_STAGE_HOST_BIT,
                             VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                             0u, 0u, NULL, 1u, &barrier, 0u, NULL);
    }

    result = vkEndCommandBuffer(command_buffer);
    if (result != VK_SUCCESS) goto cleanup;

    result = vkCreateFence(device, &fence_info, NULL, &fence);
    if (result != VK_SUCCESS) goto cleanup;

    {
        VkSubmitInfo submit_info = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1u,
            .pCommandBuffers = &command_buffer
        };
        result = vkQueueSubmit(graphics_queue, 1u, &submit_info, fence);
    }
    if (result != VK_SUCCESS) goto cleanup;

    submitted = VK_TRUE;
    result = vkWaitForFences(device, 1u, &fence, VK_TRUE, UINT64_MAX);

cleanup:
    if (submitted == VK_TRUE && result != VK_SUCCESS) {
        (void)vkDeviceWaitIdle(device);
    }
    if (fence != VK_NULL_HANDLE) vkDestroyFence(device, fence, NULL);
    if (command_pool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device, command_pool, NULL);
    }
    return result;
}

int tg_gpu_mesh_vk_upload(VkPhysicalDevice physical_device,
                          VkDevice device,
                          VkQueue graphics_queue,
                          uint32_t queue_family_index,
                          const TgMesh3D *source,
                          TgGpuMeshVk **out_mesh,
                          VkResult *out_vk_result) {
    TgMesh3D validated;
    TgGpuMeshLayout layout;
    TgVkBufferAllocation destination = {0};
    TgVkBufferAllocation staging = {0};
    TgGpuMeshVk *mesh = NULL;
    VkResult result = VK_ERROR_INITIALIZATION_FAILED;
    VkBool32 use_staging = VK_TRUE;

    set_vk_result(out_vk_result, VK_ERROR_INITIALIZATION_FAILED);

    if (physical_device == VK_NULL_HANDLE || device == VK_NULL_HANDLE ||
        graphics_queue == VK_NULL_HANDLE || source == NULL ||
        out_mesh == NULL || *out_mesh != NULL) {
        return -1;
    }

    validated = *source;
    if (tg_mesh3d_validate(&validated) != 0 ||
        validated.index_count > UINT32_MAX ||
        tg_gpu_mesh_layout_calculate((uint64_t)validated.vertex_count,
                                     (uint64_t)sizeof(TgMeshVertex3D),
                                     (uint64_t)validated.index_count,
                                     &layout) != 0) {
        return -1;
    }

    if (layout.vertex_bytes > (uint64_t)SIZE_MAX ||
        layout.index_offset > (uint64_t)SIZE_MAX ||
        layout.index_bytes > (uint64_t)SIZE_MAX ||
        layout.total_bytes > (uint64_t)SIZE_MAX) {
        return -1;
    }

    result = create_destination_buffer(physical_device, device,
                                       (VkDeviceSize)layout.total_bytes,
                                       &destination);
    if (result != VK_SUCCESS) goto cleanup;

    if ((destination.memory_properties &
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == 0u) {
        use_staging = VK_FALSE;
    }

    if (use_staging == VK_TRUE) {
        result = create_staging_buffer(physical_device, device,
                                       (VkDeviceSize)layout.total_bytes,
                                       &staging);
        if (result == VK_ERROR_FEATURE_NOT_PRESENT &&
            (destination.memory_properties &
             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0u) {
            /* Unified-memory fallback: write directly to the visible target. */
            use_staging = VK_FALSE;
            result = VK_SUCCESS;
        } else if (result != VK_SUCCESS) {
            goto cleanup;
        }

        if (use_staging == VK_TRUE) {
            result = map_and_copy(device, staging.memory,
                                  staging.allocation_size,
                                  staging.memory_properties,
                                  (VkDeviceSize)layout.vertex_bytes,
                                  (VkDeviceSize)layout.index_offset,
                                  (VkDeviceSize)layout.index_bytes,
                                  &validated);
            if (result != VK_SUCCESS) goto cleanup;
        }
    }

    if (use_staging == VK_FALSE) {
        result = map_and_copy(device, destination.memory,
                              destination.allocation_size,
                              destination.memory_properties,
                              (VkDeviceSize)layout.vertex_bytes,
                              (VkDeviceSize)layout.index_offset,
                              (VkDeviceSize)layout.index_bytes,
                              &validated);
        if (result != VK_SUCCESS) goto cleanup;
    }

    result = submit_upload(device, graphics_queue, queue_family_index,
                           &destination, &staging, &layout, use_staging);
    if (result != VK_SUCCESS) goto cleanup;

    mesh = (TgGpuMeshVk *)calloc(1u, sizeof(*mesh));
    if (mesh == NULL) {
        result = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto cleanup;
    }

    mesh->buffer = destination.buffer;
    mesh->memory = destination.memory;
    mesh->vertex_offset = (VkDeviceSize)layout.vertex_offset;
    mesh->index_offset = (VkDeviceSize)layout.index_offset;
    mesh->allocation_size = destination.allocation_size;
    mesh->index_count = (uint32_t)validated.index_count;
    mesh->memory_properties = destination.memory_properties;
    mesh->used_staging = use_staging;

    destination.buffer = VK_NULL_HANDLE;
    destination.memory = VK_NULL_HANDLE;
    *out_mesh = mesh;
    mesh = NULL;
    result = VK_SUCCESS;

cleanup:
    destroy_buffer_allocation(device, &staging);
    destroy_buffer_allocation(device, &destination);
    free(mesh);
    set_vk_result(out_vk_result, result);
    return result == VK_SUCCESS ? 0 : -1;
}

void tg_gpu_mesh_vk_destroy(VkDevice device, TgGpuMeshVk *mesh) {
    if (mesh == NULL) return;

    if (device != VK_NULL_HANDLE) {
        if (mesh->buffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(device, mesh->buffer, NULL);
        }
        if (mesh->memory != VK_NULL_HANDLE) {
            vkFreeMemory(device, mesh->memory, NULL);
        }
    }
    free(mesh);
}

VkBuffer tg_gpu_mesh_vk_buffer(const TgGpuMeshVk *mesh) {
    return mesh != NULL ? mesh->buffer : VK_NULL_HANDLE;
}

VkDeviceSize tg_gpu_mesh_vk_vertex_offset(const TgGpuMeshVk *mesh) {
    return mesh != NULL ? mesh->vertex_offset : 0u;
}

VkDeviceSize tg_gpu_mesh_vk_index_offset(const TgGpuMeshVk *mesh) {
    return mesh != NULL ? mesh->index_offset : 0u;
}

uint32_t tg_gpu_mesh_vk_index_count(const TgGpuMeshVk *mesh) {
    return mesh != NULL ? mesh->index_count : 0u;
}

VkBool32 tg_gpu_mesh_vk_used_staging(const TgGpuMeshVk *mesh) {
    return mesh != NULL ? mesh->used_staging : VK_FALSE;
}

VkMemoryPropertyFlags tg_gpu_mesh_vk_memory_properties(
    const TgGpuMeshVk *mesh) {
    return mesh != NULL ? mesh->memory_properties : 0u;
}
