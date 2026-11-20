#include "../src/camera3d.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

static int close_to(float a, float b) {
    return fabsf(a - b) < 1.0e-4f;
}

static int matrix_close(TgMat4 a, TgMat4 b) {
    int i;
    for (i = 0; i < 16; ++i) {
        if (!close_to(a.m[i], b.m[i])) return 0;
    }
    return 1;
}

int main(void) {
    TgCamera3D camera;
    TgMat4 expected_view;
    TgMat4 expected_projection;
    TgMat4 previous_view_projection;

    /* Initial camera selected for the finite 20 x 20 sea test plane. */
    assert(tg_camera3d_init(&camera,
                            tg_vec3(0.0f, 8.0f, 12.0f),
                            tg_vec3(0.0f, 0.0f, 0.0f),
                            tg_vec3(0.0f, 1.0f, 0.0f),
                            1.04719755f, 16.0f / 9.0f,
                            0.1f, 100.0f) == 0);

    assert(tg_mat4_look_at(camera.position, camera.target,
                           camera.up, &expected_view) == 0);
    assert(tg_mat4_perspective_vk(camera.vertical_fov_radians,
                                  camera.aspect_ratio,
                                  camera.near_plane,
                                  camera.far_plane,
                                  &expected_projection) == 0);
    assert(matrix_close(camera.view, expected_view));
    assert(matrix_close(camera.projection, expected_projection));
    assert(matrix_close(camera.view_projection,
                        tg_mat4_multiply(expected_projection, expected_view)));

    previous_view_projection = camera.view_projection;
    assert(tg_camera3d_set_aspect(&camera, 1.0f) == 0);
    assert(close_to(camera.aspect_ratio, 1.0f));
    assert(!matrix_close(camera.view_projection, previous_view_projection));

    previous_view_projection = camera.view_projection;
    assert(tg_camera3d_set_pose(&camera,
                                tg_vec3(0.0f, 10.0f, 15.0f),
                                tg_vec3(0.0f, 0.0f, 0.0f),
                                tg_vec3(0.0f, 1.0f, 0.0f)) == 0);
    assert(!matrix_close(camera.view_projection, previous_view_projection));

    previous_view_projection = camera.view_projection;
    assert(tg_camera3d_set_aspect(&camera, 0.0f) != 0);
    assert(matrix_close(camera.view_projection, previous_view_projection));

    assert(tg_camera3d_set_pose(&camera,
                                tg_vec3(0.0f, 0.0f, 0.0f),
                                tg_vec3(0.0f, 0.0f, 0.0f),
                                tg_vec3(0.0f, 1.0f, 0.0f)) != 0);
    assert(matrix_close(camera.view_projection, previous_view_projection));

    assert(tg_camera3d_init(&camera,
                            tg_vec3(0.0f, 0.0f, 0.0f),
                            tg_vec3(0.0f, 0.0f, -1.0f),
                            tg_vec3(0.0f, 0.0f, -1.0f),
                            1.0f, 1.0f, 0.1f, 100.0f) != 0);

    puts("3D camera tests passed");
    return 0;
}
