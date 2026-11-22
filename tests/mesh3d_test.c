#include "../src/mesh3d.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

static int close_to(float a, float b) {
    return fabsf(a - b) < 1.0e-5f;
}

int main(void) {
    TgMeshVertex3D vertices[4];
    uint32_t indices[6];
    TgMesh3D mesh;
    TgMesh3D unchanged;
    TgVec3 edge_a, edge_b, normal;

    assert(tg_mesh3d_write_plane_xz(20.0f, 20.0f,
                                    vertices, indices) == 0);
    assert(close_to(vertices[0].position.x, -10.0f));
    assert(close_to(vertices[0].position.y, 0.0f));
    assert(close_to(vertices[0].position.z, -10.0f));
    assert(close_to(vertices[2].position.x, 10.0f));
    assert(close_to(vertices[2].position.z, 10.0f));

    assert(indices[0] == 0 && indices[1] == 2 && indices[2] == 1);
    assert(indices[3] == 0 && indices[4] == 3 && indices[5] == 2);

    edge_a = tg_vec3_sub(vertices[indices[1]].position,
                         vertices[indices[0]].position);
    edge_b = tg_vec3_sub(vertices[indices[2]].position,
                         vertices[indices[0]].position);
    normal = tg_vec3_cross(edge_a, edge_b);
    assert(normal.y > 0.0f);

    assert(tg_mesh3d_init(&mesh, vertices, 4, indices, 6) == 0);
    assert(close_to(mesh.bounds_min.x, -10.0f));
    assert(close_to(mesh.bounds_min.y, 0.0f));
    assert(close_to(mesh.bounds_min.z, -10.0f));
    assert(close_to(mesh.bounds_max.x, 10.0f));
    assert(close_to(mesh.bounds_max.y, 0.0f));
    assert(close_to(mesh.bounds_max.z, 10.0f));

    unchanged = mesh;
    assert(tg_mesh3d_write_plane_xz(0.0f, 20.0f,
                                    vertices, indices) != 0);
    assert(tg_mesh3d_init(&mesh, vertices, 4, indices, 5) != 0);
    assert(mesh.vertices == unchanged.vertices);
    assert(mesh.indices == unchanged.indices);

    indices[5] = 99;
    assert(tg_mesh3d_init(&mesh, vertices, 4, indices, 6) != 0);
    indices[5] = 2;

    indices[2] = indices[1];
    assert(tg_mesh3d_init(&mesh, vertices, 4, indices, 6) != 0);
    indices[2] = 1;

    vertices[0].position.x = NAN;
    assert(tg_mesh3d_init(&mesh, vertices, 4, indices, 6) != 0);
    vertices[0].position.x = -10.0f;

    assert(tg_mesh3d_init(&mesh, vertices, 4, indices, 6) == 0);
    assert(tg_mesh3d_validate(&mesh) == 0);

    puts("3D mesh tests passed");
    return 0;
}
