#ifndef TORNIA_CAMERA3D_H
#define TORNIA_CAMERA3D_H

#include "math3d.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    TgVec3 position;
    TgVec3 target;
    TgVec3 up;
    float vertical_fov_radians;
    float aspect_ratio;
    float near_plane;
    float far_plane;
    TgMat4 view;
    TgMat4 projection;
    TgMat4 view_projection;
} TgCamera3D;

/* Initializes a right-handed, Y-up camera looking along local -Z.
 * Returns 0 on success and -1 for invalid arguments or a degenerate pose.
 */
int tg_camera3d_init(TgCamera3D *camera,
                     TgVec3 position,
                     TgVec3 target,
                     TgVec3 up,
                     float vertical_fov_radians,
                     float aspect_ratio,
                     float near_plane,
                     float far_plane);

/* Changes the camera pose and recalculates its matrices. */
int tg_camera3d_set_pose(TgCamera3D *camera,
                         TgVec3 position,
                         TgVec3 target,
                         TgVec3 up);

/* Updates projection for a resized viewport. */
int tg_camera3d_set_aspect(TgCamera3D *camera, float aspect_ratio);

/* Recalculates view, Vulkan projection, and projection * view. */
int tg_camera3d_update(TgCamera3D *camera);

#ifdef __cplusplus
}
#endif
#endif
