#include "mesh3d.h"

#include <math.h>

#define TG_MESH_EPSILON_SQUARED 1.0e-12f

static int valid_vec3(TgVec3 value) {
    return isfinite(value.x) && isfinite(value.y) && isfinite(value.z);
}

static TgVec3 vec3_min(TgVec3 a, TgVec3 b) {
    return tg_vec3(fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z));
}

static TgVec3 vec3_max(TgVec3 a, TgVec3 b) {
    return tg_vec3(fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z));
}

int tg_mesh3d_validate(TgMesh3D *mesh) {
    size_t i;
    TgVec3 bounds_min;
    TgVec3 bounds_max;

    if (mesh == NULL || mesh->vertices == NULL || mesh->indices == NULL ||
        mesh->vertex_count < 3 || mesh->index_count < 3 ||
        mesh->index_count % 3 != 0) {
        return -1;
    }

    if (!valid_vec3(mesh->vertices[0].position)) return -1;
    bounds_min = mesh->vertices[0].position;
    bounds_max = mesh->vertices[0].position;

    for (i = 1; i < mesh->vertex_count; ++i) {
        TgVec3 position = mesh->vertices[i].position;
        if (!valid_vec3(position)) return -1;
        bounds_min = vec3_min(bounds_min, position);
        bounds_max = vec3_max(bounds_max, position);
    }

    for (i = 0; i < mesh->index_count; i += 3) {
        uint32_t ia = mesh->indices[i];
        uint32_t ib = mesh->indices[i + 1];
        uint32_t ic = mesh->indices[i + 2];
        TgVec3 edge_ab, edge_ac, area_vector;
        float area_squared;

        if ((size_t)ia >= mesh->vertex_count ||
            (size_t)ib >= mesh->vertex_count ||
            (size_t)ic >= mesh->vertex_count) {
            return -1;
        }

        edge_ab = tg_vec3_sub(mesh->vertices[ib].position,
                              mesh->vertices[ia].position);
        edge_ac = tg_vec3_sub(mesh->vertices[ic].position,
                              mesh->vertices[ia].position);
        area_vector = tg_vec3_cross(edge_ab, edge_ac);
        area_squared = tg_vec3_dot(area_vector, area_vector);

        if (!isfinite(area_squared) ||
            area_squared <= TG_MESH_EPSILON_SQUARED) {
            return -1;
        }
    }

    mesh->bounds_min = bounds_min;
    mesh->bounds_max = bounds_max;
    return 0;
}

int tg_mesh3d_init(TgMesh3D *mesh,
                   const TgMeshVertex3D *vertices,
                   size_t vertex_count,
                   const uint32_t *indices,
                   size_t index_count) {
    TgMesh3D candidate;

    if (mesh == NULL) return -1;

    candidate.vertices = vertices;
    candidate.vertex_count = vertex_count;
    candidate.indices = indices;
    candidate.index_count = index_count;
    candidate.bounds_min = tg_vec3(0.0f, 0.0f, 0.0f);
    candidate.bounds_max = tg_vec3(0.0f, 0.0f, 0.0f);

    if (tg_mesh3d_validate(&candidate) != 0) return -1;

    *mesh = candidate;
    return 0;
}

int tg_mesh3d_write_plane_xz(float width,
                             float depth,
                             TgMeshVertex3D vertices[4],
                             uint32_t indices[6]) {
    float half_width;
    float half_depth;

    if (vertices == NULL || indices == NULL ||
        !isfinite(width) || !isfinite(depth) ||
        width <= 0.0f || depth <= 0.0f) {
        return -1;
    }

    half_width = width * 0.5f;
    half_depth = depth * 0.5f;
    if (!isfinite(half_width) || !isfinite(half_depth)) return -1;

    vertices[0].position = tg_vec3(-half_width, 0.0f, -half_depth);
    vertices[1].position = tg_vec3( half_width, 0.0f, -half_depth);
    vertices[2].position = tg_vec3( half_width, 0.0f,  half_depth);
    vertices[3].position = tg_vec3(-half_width, 0.0f,  half_depth);

    /* Counter-clockwise from +Y, yielding an upward-facing normal. */
    indices[0] = 0; indices[1] = 2; indices[2] = 1;
    indices[3] = 0; indices[4] = 3; indices[5] = 2;
    return 0;
}
