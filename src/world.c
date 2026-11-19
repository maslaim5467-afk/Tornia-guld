#include "world.h"

#define WORLD_MIN_ZOOM 0.75f
#define WORLD_MAX_ZOOM 2.50f

static uint32_t hash32(uint32_t x) {
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}

static uint32_t tile_hash(uint32_t seed, uint32_t x, uint32_t y, uint32_t salt) {
    uint32_t v = seed ^ (x * 0x9e3779b9u) ^ (y * 0x85ebca6bu) ^ salt;
    return hash32(v);
}

static float clampf(float value, float low, float high) {
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

void world_generate(World *world, uint32_t seed) {
    uint32_t x, y;
    if (!world) return;

    world->seed = seed ? seed : 0x4f4c5245u;

    for (y = 0; y < WORLD_HEIGHT; ++y) {
        for (x = 0; x < WORLD_WIDTH; ++x) {
            WorldTile *tile = &world->tiles[y][x];
            uint32_t noise = tile_hash(world->seed, x, y, 0x13579bdu);
            uint32_t cluster = tile_hash(world->seed, x / 3u, y / 3u, 0x2468aceu);
            uint32_t density = (noise >> 24) + ((cluster >> 27) * 3u);

            tile->overlay_count = 0;
            tile->overlays[0] = 0;
            tile->overlays[1] = 0;
            tile->variation = (uint8_t)((noise >> 20) & 3u);
            tile->reserved = 0;

            /* Sparse, deterministic natural dressing; IDs are 1-based asset IDs. */
            if ((density % 100u) < 24u) {
                uint8_t first = (uint8_t)(1u + ((noise >> 8) % WORLD_OVERLAY_COUNT));
                tile->overlays[tile->overlay_count++] = first;

                /* A second detail is deliberately uncommon to avoid visual clutter. */
                if (((noise >> 16) & 0xffu) < 22u) {
                    uint8_t second = (uint8_t)(1u + ((noise >> 2) % WORLD_OVERLAY_COUNT));
                    if (second == first)
                        second = (uint8_t)((second % WORLD_OVERLAY_COUNT) + 1u);
                    tile->overlays[tile->overlay_count++] = second;
                }
            }
        }
    }
}

void world_camera_reset(WorldCamera *camera) {
    if (!camera) return;
    camera->x = 0.0f;
    camera->y = 0.0f;
    camera->zoom = 1.0f;
}

void world_camera_clamp(WorldCamera *camera,
                        float viewport_width, float viewport_height) {
    float world_width, world_height, visible_width, visible_height;
    if (!camera || viewport_width <= 0.0f || viewport_height <= 0.0f) return;

    camera->zoom = clampf(camera->zoom, WORLD_MIN_ZOOM, WORLD_MAX_ZOOM);
    world_width = (float)WORLD_WIDTH * WORLD_TILE_SIZE;
    world_height = (float)WORLD_HEIGHT * WORLD_TILE_SIZE;
    visible_width = viewport_width / camera->zoom;
    visible_height = viewport_height / camera->zoom;

    if (visible_width >= world_width) camera->x = (world_width - visible_width) * 0.5f;
    else camera->x = clampf(camera->x, 0.0f, world_width - visible_width);

    if (visible_height >= world_height) camera->y = (world_height - visible_height) * 0.5f;
    else camera->y = clampf(camera->y, 0.0f, world_height - visible_height);
}

void world_camera_pan(WorldCamera *camera, float dx, float dy,
                      float viewport_width, float viewport_height) {
    if (!camera) return;
    camera->x -= dx / camera->zoom;
    camera->y -= dy / camera->zoom;
    world_camera_clamp(camera, viewport_width, viewport_height);
}

float world_camera_world_x(const WorldCamera *camera, float screen_x) {
    if (!camera) return screen_x;
    return camera->x + screen_x / camera->zoom;
}

float world_camera_world_y(const WorldCamera *camera, float screen_y) {
    if (!camera) return screen_y;
    return camera->y + screen_y / camera->zoom;
}
