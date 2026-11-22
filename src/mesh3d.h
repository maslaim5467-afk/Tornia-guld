#ifndef TORNIA_MESH3D_H
#define TORNIA_MESH3D_H

#include "math3d.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    TgVec3 position;
} TgMeshVertex3D;

/*
 * CPU-side mesh view. Vertex and index arrays are borrowed, not owned.
 * The caller must keep both arrays alive and unchanged while the mesh is used.
 * Indices describe triangles (three uint32_t indices per triangle).
 */
typedef struct {
    const TgMeshVertex3D *vertices;
    size_t vertex_count;
    const uint32_t *indices;
    size_t index_count;
    TgVec3 bounds_min;
    TgVec3 bounds_max;
} TgMesh3D;

/*
 * Validates and initializes a non-owning mesh view.
 * Rejects non-finite vertices, out-of-range indices, incomplete triangles,
 * and degenerate triangles. Returns 0 on success, -1 on invalid input.
 * On failure, *mesh is left unchanged.
 */
int tg_mesh3d_init(TgMesh3D *mesh,
                   const TgMeshVertex3D *vertices,
                   size_t vertex_count,
                   const uint32_t *indices,
                   size_t index_count);

/* Revalidates borrowed source data and refreshes its axis-aligned bounds. */
int tg_mesh3d_validate(TgMesh3D *mesh);

/*
 * Writes a centered, +Y-facing rectangle in the XZ plane.
 * Output storage must provide four vertices and six indices.
 * Width and depth must be finite and greater than zero.
 */
int tg_mesh3d_write_plane_xz(float width,
                             float depth,
                             TgMeshVertex3D vertices[4],
                             uint32_t indices[6]);

#ifdef __cplusplus
}
#endif
#endif
