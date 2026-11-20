#ifndef TORNIA_MATH3D_H
#define TORNIA_MATH3D_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { float x, y, z; } TgVec3;
typedef struct { float m[16]; } TgMat4;

TgVec3 tg_vec3(float x, float y, float z);
TgVec3 tg_vec3_add(TgVec3 a, TgVec3 b);
TgVec3 tg_vec3_sub(TgVec3 a, TgVec3 b);
TgVec3 tg_vec3_scale(TgVec3 v, float scalar);
float tg_vec3_dot(TgVec3 a, TgVec3 b);
TgVec3 tg_vec3_cross(TgVec3 a, TgVec3 b);
float tg_vec3_length(TgVec3 v);
int tg_vec3_normalize(TgVec3 v, TgVec3 *out);

TgMat4 tg_mat4_identity(void);
TgMat4 tg_mat4_multiply(TgMat4 a, TgMat4 b);
TgMat4 tg_mat4_translation(TgVec3 translation);
TgMat4 tg_mat4_scale(TgVec3 scale);
/* Right-handed view matrix; camera looks along its local -Z axis. */
int tg_mat4_look_at(TgVec3 eye, TgVec3 center, TgVec3 up, TgMat4 *out);
/* Right-handed perspective projection with Vulkan's [0, 1] depth range.
 * fov_y_radians must be between 0 and pi; aspect and near/far must be positive,
 * with far_plane > near_plane.
 */
int tg_mat4_perspective_vk(float fov_y_radians, float aspect,
                           float near_plane, float far_plane, TgMat4 *out);

#ifdef __cplusplus
}
#endif
#endif
