#include "sea_camera.h"

#include <math.h>

TgVec3 tg_sea_camera_forward(float yaw_radians, float pitch_radians) {
    float cos_pitch = cosf(pitch_radians);

    return tg_vec3(sinf(yaw_radians) * cos_pitch,
                   sinf(pitch_radians),
                   -cosf(yaw_radians) * cos_pitch);
}

int tg_sea_camera_rebuild(TgCamera3D *camera,
                          float yaw_radians,
                          float pitch_radians) {
    TgVec3 forward;
    TgVec3 target;

    if (camera == NULL) {
        return -1;
    }

    forward = tg_sea_camera_forward(yaw_radians, pitch_radians);
    target = tg_vec3(camera->position.x + forward.x,
                     camera->position.y + forward.y,
                     camera->position.z + forward.z);

    return tg_camera3d_set_pose(camera,
                                camera->position,
                                target,
                                tg_vec3(0.0f, 1.0f, 0.0f));
}

int tg_sea_camera_translate(TgCamera3D *camera,
                            float yaw_radians,
                            float pitch_radians,
                            float right_amount,
                            float up_amount,
                            float forward_amount) {
    float cos_yaw;
    float sin_yaw;
    float cos_pitch;
    float sin_pitch;
    TgVec3 right;
    TgVec3 up;
    TgVec3 forward;

    if (camera == NULL) {
        return -1;
    }

    cos_yaw = cosf(yaw_radians);
    sin_yaw = sinf(yaw_radians);
    cos_pitch = cosf(pitch_radians);
    sin_pitch = sinf(pitch_radians);

    right = tg_vec3(cos_yaw, 0.0f, sin_yaw);
    up = tg_vec3(-sin_yaw * sin_pitch, cos_pitch, cos_yaw * sin_pitch);
    forward = tg_sea_camera_forward(yaw_radians, pitch_radians);

    camera->position.x += right.x * right_amount +
                          up.x * up_amount +
                          forward.x * forward_amount;
    camera->position.y += right.y * right_amount +
                          up.y * up_amount +
                          forward.y * forward_amount;
    camera->position.z += right.z * right_amount +
                          up.z * up_amount +
                          forward.z * forward_amount;

    return tg_sea_camera_rebuild(camera, yaw_radians, pitch_radians);
}
