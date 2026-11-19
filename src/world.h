#ifndef TORNIA_WORLD_H
#define TORNIA_WORLD_H

#include <stdint.h>

#define WORLD_WIDTH 30u
#define WORLD_HEIGHT 30u
#define WORLD_TILE_SIZE 128.0f
#define WORLD_MAX_OVERLAYS 2u
#define WORLD_OVERLAY_COUNT 13u

typedef struct {
    uint8_t overlay_count;
    uint8_t overlays[WORLD_MAX_OVERLAYS];
    uint8_t variation;
    uint8_t reserved;
} WorldTile;

typedef struct {
    WorldTile tiles[WORLD_HEIGHT][WORLD_WIDTH];
    uint32_t seed;
} World;

typedef struct {
    float x;
    float y;
    float zoom;
} WorldCamera;

void world_generate(World *world, uint32_t seed);
void world_camera_reset(WorldCamera *camera);
void world_camera_pan(WorldCamera *camera, float dx, float dy,
                      float viewport_width, float viewport_height);
void world_camera_clamp(WorldCamera *camera,
                        float viewport_width, float viewport_height);
void world_camera_follow(WorldCamera *camera, float target_x, float target_y,
                         float anchor_x, float anchor_y,
                         float viewport_width, float viewport_height,
                         float delta_seconds, float responsiveness);
float world_camera_world_x(const WorldCamera *camera, float screen_x);
float world_camera_world_y(const WorldCamera *camera, float screen_y);

#endif
