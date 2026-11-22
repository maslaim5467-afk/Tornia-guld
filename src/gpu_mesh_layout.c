#include "gpu_mesh_layout.h"

#include <stddef.h>

#define TG_GPU_MESH_INDEX_ALIGNMENT ((uint64_t)sizeof(uint32_t))

static int checked_multiply_u64(uint64_t a, uint64_t b, uint64_t *out) {
    if (out == NULL || (b != 0u && a > UINT64_MAX / b)) {
        return -1;
    }

    *out = a * b;
    return 0;
}

static int checked_add_u64(uint64_t a, uint64_t b, uint64_t *out) {
    if (out == NULL || a > UINT64_MAX - b) {
        return -1;
    }

    *out = a + b;
    return 0;
}

int tg_gpu_mesh_layout_calculate(uint64_t vertex_count,
                                 uint64_t vertex_stride,
                                 uint64_t index_count,
                                 TgGpuMeshLayout *out_layout) {
    TgGpuMeshLayout candidate;
    uint64_t padding;
    uint64_t remainder;

    if (out_layout == NULL ||
        vertex_count < 3u ||
        vertex_stride == 0u ||
        index_count < 3u ||
        index_count % 3u != 0u) {
        return -1;
    }

    candidate.vertex_offset = 0u;
    if (checked_multiply_u64(vertex_count, vertex_stride,
                             &candidate.vertex_bytes) != 0) {
        return -1;
    }

    remainder = candidate.vertex_bytes % TG_GPU_MESH_INDEX_ALIGNMENT;
    padding = remainder == 0u ? 0u :
              TG_GPU_MESH_INDEX_ALIGNMENT - remainder;
    if (checked_add_u64(candidate.vertex_bytes, padding,
                        &candidate.index_offset) != 0) {
        return -1;
    }

    if (checked_multiply_u64(index_count, sizeof(uint32_t),
                             &candidate.index_bytes) != 0) {
        return -1;
    }

    if (checked_add_u64(candidate.index_offset, candidate.index_bytes,
                        &candidate.total_bytes) != 0) {
        return -1;
    }

    *out_layout = candidate;
    return 0;
}
