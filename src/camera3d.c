#include "camera3d.h"

#include <math.h>
#include <stddef.h>

static int valid_vec3(TgVec3 value) {
    return isfinite(value.x) && isfinite(value.y) && isfinite(value.z);
}

int tg_camera3d_update(TgCamera3D *camera) {
    TgMat4 view;
    TgMat4 projection;

    if (camera == NULL ||
        !valid_vec3(camera->position) ||
        !valid_vec3(camera->target) ||
        !valid_vec3(camera->up)) {
        return -1;
    }

    if (tg_mat4_look_at(camera->position, camera->target,
                        camera->up, &view) != 0) {
        return -1;
    }

    if (tg_mat4_perspective_vk(camera->vertical_fov_radians,
                              camera->aspect_ratio,
                              camera->near_plane,
                              camera->far_plane,
                              &projection) != 0) {
        return -1;
    }

    camera->view = view;
    camera->projection = projection;
    camera->view_projection = tg_mat4_multiply(projection, view);
    return 0;
}

int tg_camera3d_init(TgCamera3D *camera,
                     TgVec3 position,
                     TgVec3 target,
                     TgVec3 up,
                     float vertical_fov_radians,
                     float aspect_ratio,
                     float near_plane,
                     float far_plane) {
    TgCamera3D candidate;

    if (camera == NULL) return -1;

    candidate.position = position;
    candidate.target = target;
    candidate.up = up;
    candidate.vertical_fov_radians = vertical_fov_radians;
    candidate.aspect_ratio = aspect_ratio;
    candidate.near_plane = near_plane;
    candidate.far_plane = far_plane;

    if (tg_camera3d_update(&candidate) != 0) return -1;

    *camera = candidate;
    return 0;
}

int tg_camera3d_set_pose(TgCamera3D *camera,
                         TgVec3 position,
                         TgVec3 target,
                         TgVec3 up) {
    TgCamera3D candidate;

    if (camera == NULL) return -1;

    candidate = *camera;
    candidate.position = position;
    candidate.target = target;
    candidate.up = up;

    if (tg_camera3d_update(&candidate) != 0) return -1;

    *camera = candidate;
    return 0;
}

int tg_camera3d_set_aspect(TgCamera3D *camera, float aspect_ratio) {
    TgCamera3D candidate;

    if (camera == NULL || !isfinite(aspect_ratio) || aspect_ratio <= 0.0f)
        return -1;

    candidate = *camera;
    candidate.aspect_ratio = aspect_ratio;

    if (tg_camera3d_update(&candidate) != 0) return -1;

    *camera = candidate;
    return 0;
}
