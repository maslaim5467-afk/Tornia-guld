#include "math3d.h"

#include <math.h>
#include <stddef.h>

#define TG_EPSILON 1.0e-6f
#define TG_PI 3.14159265358979323846f

TgVec3 tg_vec3(float x, float y, float z) {
    TgVec3 v = {x, y, z};
    return v;
}

TgVec3 tg_vec3_add(TgVec3 a, TgVec3 b) {
    return tg_vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

TgVec3 tg_vec3_sub(TgVec3 a, TgVec3 b) {
    return tg_vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

TgVec3 tg_vec3_scale(TgVec3 v, float scalar) {
    return tg_vec3(v.x * scalar, v.y * scalar, v.z * scalar);
}

float tg_vec3_dot(TgVec3 a, TgVec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

TgVec3 tg_vec3_cross(TgVec3 a, TgVec3 b) {
    return tg_vec3(a.y * b.z - a.z * b.y,
                   a.z * b.x - a.x * b.z,
                   a.x * b.y - a.y * b.x);
}

float tg_vec3_length(TgVec3 v) {
    return sqrtf(tg_vec3_dot(v, v));
}

int tg_vec3_normalize(TgVec3 v, TgVec3 *out) {
    float length;
    if (out == NULL) return -1;
    length = tg_vec3_length(v);
    if (!isfinite(length) || length <= TG_EPSILON) return -1;
    *out = tg_vec3_scale(v, 1.0f / length);
    return 0;
}

TgMat4 tg_mat4_identity(void) {
    TgMat4 result = {{0.0f}};
    result.m[0] = 1.0f;
    result.m[5] = 1.0f;
    result.m[10] = 1.0f;
    result.m[15] = 1.0f;
    return result;
}

TgMat4 tg_mat4_multiply(TgMat4 a, TgMat4 b) {
    TgMat4 result = {{0.0f}};
    size_t column, row, k;
    for (column = 0; column < 4; ++column) {
        for (row = 0; row < 4; ++row) {
            for (k = 0; k < 4; ++k) {
                result.m[column * 4 + row] +=
                    a.m[k * 4 + row] * b.m[column * 4 + k];
            }
        }
    }
    return result;
}

TgMat4 tg_mat4_translation(TgVec3 translation) {
    TgMat4 result = tg_mat4_identity();
    result.m[12] = translation.x;
    result.m[13] = translation.y;
    result.m[14] = translation.z;
    return result;
}

TgMat4 tg_mat4_scale(TgVec3 scale) {
    TgMat4 result = {{0.0f}};
    result.m[0] = scale.x;
    result.m[5] = scale.y;
    result.m[10] = scale.z;
    result.m[15] = 1.0f;
    return result;
}

int tg_mat4_look_at(TgVec3 eye, TgVec3 center, TgVec3 up, TgMat4 *out) {
    TgVec3 forward, side, corrected_up;
    if (out == NULL) return -1;
    if (tg_vec3_normalize(tg_vec3_sub(center, eye), &forward) != 0)
        return -1;
    if (tg_vec3_normalize(tg_vec3_cross(forward, up), &side) != 0)
        return -1;
    corrected_up = tg_vec3_cross(side, forward);

    *out = tg_mat4_identity();
    out->m[0] = side.x;
    out->m[1] = corrected_up.x;
    out->m[2] = -forward.x;
    out->m[4] = side.y;
    out->m[5] = corrected_up.y;
    out->m[6] = -forward.y;
    out->m[8] = side.z;
    out->m[9] = corrected_up.z;
    out->m[10] = -forward.z;
    out->m[12] = -tg_vec3_dot(side, eye);
    out->m[13] = -tg_vec3_dot(corrected_up, eye);
    out->m[14] = tg_vec3_dot(forward, eye);
    return 0;
}

int tg_mat4_perspective_vk(float fov_y_radians, float aspect,
                           float near_plane, float far_plane, TgMat4 *out) {
    float tangent;
    TgMat4 result = {{0.0f}};
    if (out == NULL || !isfinite(fov_y_radians) || !isfinite(aspect) ||
        !isfinite(near_plane) || !isfinite(far_plane) ||
        fov_y_radians <= 0.0f || fov_y_radians >= TG_PI ||
        aspect <= 0.0f || near_plane <= 0.0f || far_plane <= near_plane)
        return -1;

    tangent = tanf(fov_y_radians * 0.5f);
    if (!isfinite(tangent) || tangent <= TG_EPSILON) return -1;

    result.m[0] = 1.0f / (aspect * tangent);
    result.m[5] = 1.0f / tangent;
    result.m[10] = far_plane / (near_plane - far_plane);
    result.m[11] = -1.0f;
    result.m[14] = (far_plane * near_plane) / (near_plane - far_plane);
    *out = result;
    return 0;
}
