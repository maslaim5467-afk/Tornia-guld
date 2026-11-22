#include "../src/sea_camera.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

static int close_to(float a, float b) {
    return fabsf(a - b) < 1.0e-4f;
}

int main(void) {
    TgCamera3D camera;
    TgVec3 forward;

    forward = tg_sea_camera_forward(0.0f, 0.0f);
    assert(close_to(forward.x, 0.0f));
    assert(close_to(forward.y, 0.0f));
    assert(close_to(forward.z, -1.0f));

    assert(tg_camera3d_init(&camera,
                            tg_vec3(0.0f, 0.0f, 0.0f),
                            tg_vec3(0.0f, 0.0f, -1.0f),
                            tg_vec3(0.0f, 1.0f, 0.0f),
                            0.92f, 16.0f / 9.0f, 0.1f, 2500.0f) == 0);

    assert(tg_sea_camera_translate(&camera, 0.0f, 0.0f,
                                   2.0f, 3.0f, 4.0f) == 0);
    assert(close_to(camera.position.x, 2.0f));
    assert(close_to(camera.position.y, 3.0f));
    assert(close_to(camera.position.z, -4.0f));
    assert(close_to(camera.target.x, 2.0f));
    assert(close_to(camera.target.y, 3.0f));
    assert(close_to(camera.target.z, -5.0f));

    assert(tg_sea_camera_rebuild(&camera, 0.0f, 0.0f) == 0);
    assert(close_to(camera.target.x, camera.position.x));
    assert(close_to(camera.target.y, camera.position.y));
    assert(close_to(camera.target.z, camera.position.z - 1.0f));

    assert(tg_sea_camera_rebuild(NULL, 0.0f, 0.0f) != 0);
    assert(tg_sea_camera_translate(NULL, 0.0f, 0.0f,
                                   1.0f, 0.0f, 0.0f) != 0);

    puts("Sea camera tests passed");
    return 0;
}
