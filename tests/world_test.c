#include "../src/world.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    World a, b;
    WorldCamera camera;
    unsigned int x, y, overlays = 0;

    world_generate(&a, 0x12345678u);
    world_generate(&b, 0x12345678u);
    assert(memcmp(&a, &b, sizeof(World)) == 0);

    world_generate(&b, 0x87654321u);
    assert(memcmp(&a, &b, sizeof(World)) != 0);

    for (y = 0; y < WORLD_HEIGHT; ++y) {
        for (x = 0; x < WORLD_WIDTH; ++x) {
            const WorldTile *tile = &a.tiles[y][x];
            assert(tile->overlay_count <= WORLD_MAX_OVERLAYS);
            for (unsigned int i = 0; i < tile->overlay_count; ++i) {
                assert(tile->overlays[i] >= 1u);
                assert(tile->overlays[i] <= WORLD_OVERLAY_COUNT);
                ++overlays;
            }
        }
    }
    assert(overlays > 0u);

    world_camera_reset(&camera);
    camera.x = -500.0f;
    camera.y = 9000.0f;
    world_camera_clamp(&camera, 800.0f, 450.0f);
    assert(camera.x >= 0.0f);
    assert(camera.y >= 0.0f);

    world_camera_pan(&camera, -100000.0f, -100000.0f, 800.0f, 450.0f);
    assert(camera.x <= WORLD_WIDTH * WORLD_TILE_SIZE);
    assert(camera.y <= WORLD_HEIGHT * WORLD_TILE_SIZE);
    assert(world_camera_world_x(&camera, 0.0f) == camera.x);
    assert(world_camera_world_y(&camera, 0.0f) == camera.y);

    printf("world tests passed; overlays=%u\n", overlays);
    return 0;
}
