#ifndef TORNIA_SEA_CAMERA_H
#define TORNIA_SEA_CAMERA_H

#include "camera3d.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Returns the normalized forward direction for the sea preview camera angles. */
TgVec3 tg_sea_camera_forward(float yaw_radians, float pitch_radians);

/* Rebuilds the camera view using its current position and the supplied angles. */
int tg_sea_camera_rebuild(TgCamera3D *camera,
                          float yaw_radians,
                          float pitch_radians);

/* Translates the camera in its local right/up/forward basis, then rebuilds it. */
int tg_sea_camera_translate(TgCamera3D *camera,
                            float yaw_radians,
                            float pitch_radians,
                            float right_amount,
                            float up_amount,
                            float forward_amount);

#ifdef __cplusplus
}
#endif

#endif
