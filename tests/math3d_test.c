#include "../src/math3d.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

static int close_to(float a, float b) {
    return fabsf(a - b) < 1.0e-4f;
}

int main(void) {
    TgVec3 x = tg_vec3(1.0f, 0.0f, 0.0f);
    TgVec3 y = tg_vec3(0.0f, 1.0f, 0.0f);
    TgVec3 z = tg_vec3_cross(x, y);
    TgVec3 normalized;
    TgMat4 identity = tg_mat4_identity();
    TgMat4 translation = tg_mat4_translation(tg_vec3(3.0f, 4.0f, 5.0f));
    TgMat4 product = tg_mat4_multiply(identity, translation);
    TgMat4 view, projection;

    assert(close_to(z.x, 0.0f) && close_to(z.y, 0.0f) &&
           close_to(z.z, 1.0f));
    assert(close_to(tg_vec3_dot(x, y), 0.0f));
    assert(tg_vec3_normalize(tg_vec3(0.0f, 3.0f, 4.0f), &normalized) == 0);
    assert(close_to(tg_vec3_length(normalized), 1.0f));
    assert(tg_vec3_normalize(tg_vec3(0.0f, 0.0f, 0.0f), &normalized) != 0);

    assert(close_to(product.m[12], 3.0f));
    assert(close_to(product.m[13], 4.0f));
    assert(close_to(product.m[14], 5.0f));

    assert(tg_mat4_look_at(tg_vec3(0.0f, 0.0f, 3.0f),
                           tg_vec3(0.0f, 0.0f, 0.0f),
                           tg_vec3(0.0f, 1.0f, 0.0f), &view) == 0);
    assert(close_to(view.m[14], -3.0f));
    assert(tg_mat4_look_at(tg_vec3(0.0f, 0.0f, 0.0f),
                           tg_vec3(0.0f, 0.0f, 0.0f),
                           tg_vec3(0.0f, 1.0f, 0.0f), &view) != 0);

    assert(tg_mat4_perspective_vk(1.04719755f, 16.0f / 9.0f,
                                  0.1f, 100.0f, &projection) == 0);
    assert(projection.m[0] > 0.0f && projection.m[5] > 0.0f);
    assert(close_to(projection.m[11], -1.0f));
    assert(tg_mat4_perspective_vk(1.0f, 0.0f, 0.1f, 100.0f,
                                  &projection) != 0);
    assert(tg_mat4_perspective_vk(1.0f, 1.0f, 10.0f, 1.0f,
                                  &projection) != 0);

    puts("3D math foundation tests passed");
    return 0;
}
