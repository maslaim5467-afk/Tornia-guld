#include "../src/gpu_mesh_layout.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static void assert_layout_equal(TgGpuMeshLayout a, TgGpuMeshLayout b) {
    assert(a.vertex_offset == b.vertex_offset);
    assert(a.vertex_bytes == b.vertex_bytes);
    assert(a.index_offset == b.index_offset);
    assert(a.index_bytes == b.index_bytes);
    assert(a.total_bytes == b.total_bytes);
}

static void test_plane_layout(void) {
    TgGpuMeshLayout layout = {0u, 0u, 0u, 0u, 0u};

    assert(tg_gpu_mesh_layout_calculate(4u, 12u, 6u, &layout) == 0);
    assert(layout.vertex_offset == 0u);
    assert(layout.vertex_bytes == 48u);
    assert(layout.index_offset == 48u);
    assert(layout.index_bytes == 24u);
    assert(layout.total_bytes == 72u);
}

static void test_index_alignment(void) {
    TgGpuMeshLayout layout = {0u, 0u, 0u, 0u, 0u};

    assert(tg_gpu_mesh_layout_calculate(3u, 13u, 3u, &layout) == 0);
    assert(layout.vertex_bytes == 39u);
    assert(layout.index_offset == 40u);
    assert(layout.index_bytes == 12u);
    assert(layout.total_bytes == 52u);
}

static void test_invalid_arguments(void) {
    TgGpuMeshLayout layout = {1u, 2u, 3u, 4u, 5u};
    const TgGpuMeshLayout original = layout;

    assert(tg_gpu_mesh_layout_calculate(2u, 12u, 3u, &layout) == -1);
    assert_layout_equal(layout, original);
    assert(tg_gpu_mesh_layout_calculate(3u, 0u, 3u, &layout) == -1);
    assert_layout_equal(layout, original);
    assert(tg_gpu_mesh_layout_calculate(3u, 12u, 2u, &layout) == -1);
    assert_layout_equal(layout, original);
    assert(tg_gpu_mesh_layout_calculate(3u, 12u, 4u, &layout) == -1);
    assert_layout_equal(layout, original);
    assert(tg_gpu_mesh_layout_calculate(3u, 12u, 3u, NULL) == -1);
}

static void test_vertex_multiplication_overflow(void) {
    TgGpuMeshLayout layout = {7u, 8u, 9u, 10u, 11u};
    const TgGpuMeshLayout original = layout;

    assert(tg_gpu_mesh_layout_calculate(UINT64_MAX / 2u + 1u,
                                        2u, 3u, &layout) == -1);
    assert_layout_equal(layout, original);
}

static void test_alignment_addition_overflow(void) {
    TgGpuMeshLayout layout = {7u, 8u, 9u, 10u, 11u};
    const TgGpuMeshLayout original = layout;

    assert(tg_gpu_mesh_layout_calculate(UINT64_MAX, 1u, 3u, &layout) == -1);
    assert_layout_equal(layout, original);
}

static void test_index_multiplication_overflow(void) {
    TgGpuMeshLayout layout = {7u, 8u, 9u, 10u, 11u};
    const TgGpuMeshLayout original = layout;

    assert(tg_gpu_mesh_layout_calculate(3u, 12u, UINT64_MAX, &layout) == -1);
    assert_layout_equal(layout, original);
}

static void test_total_size_addition_overflow(void) {
    TgGpuMeshLayout layout = {7u, 8u, 9u, 10u, 11u};
    const TgGpuMeshLayout original = layout;

    /* indexOffset = 4; indexBytes = UINT64_MAX - 3 (still divisible by 4). */
    assert(tg_gpu_mesh_layout_calculate(3u, 1u,
                                        (UINT64_MAX - 3u) / 4u,
                                        &layout) == -1);
    assert_layout_equal(layout, original);
}

int main(void) {
    test_plane_layout();
    test_index_alignment();
    test_invalid_arguments();
    test_vertex_multiplication_overflow();
    test_alignment_addition_overflow();
    test_index_multiplication_overflow();
    test_total_size_addition_overflow();
    puts("GPU mesh layout tests passed");
    return 0;
}
