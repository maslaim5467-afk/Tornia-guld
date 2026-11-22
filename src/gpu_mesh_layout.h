#ifndef TORNIA_GPU_MESH_LAYOUT_H
#define TORNIA_GPU_MESH_LAYOUT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint64_t vertex_offset;
    uint64_t vertex_bytes;
    uint64_t index_offset;
    uint64_t index_bytes;
    uint64_t total_bytes;
} TgGpuMeshLayout;

/*
 * Calculates the packed layout for one vertex region followed by uint32 indices.
 * The index region is aligned to sizeof(uint32_t).
 *
 * Counts and stride must describe a triangle mesh with at least three vertices
 * and three indices. Returns 0 on success and -1 on invalid input or overflow.
 * On failure, *out_layout is left unchanged.
 */
int tg_gpu_mesh_layout_calculate(uint64_t vertex_count,
                                 uint64_t vertex_stride,
                                 uint64_t index_count,
                                 TgGpuMeshLayout *out_layout);

#ifdef __cplusplus
}
#endif

#endif
