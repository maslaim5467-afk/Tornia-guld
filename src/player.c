#include "player.h"

#include <math.h>
#include <stddef.h>

#define PLAYER_INPUT_DEADZONE 0.12f
#define PLAYER_RUN_THRESHOLD 0.82f
#define PLAYER_WORLD_MARGIN 8.0f

static float clampf(float value, float low, float high) {
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

void player_init(Player *player, float world_x, float world_y) {
    if (!player) return;
    player->x = world_x;
    player->y = world_y;
    player->velocity_x = 0.0f;
    player->velocity_y = 0.0f;
    player->input_x = 0.0f;
    player->input_y = 0.0f;
    player->input_strength = 0.0f;
    player->walk_speed = 145.0f;
    player->run_speed = 225.0f;
    player->facing = PLAYER_FACING_DOWN;
    player->motion = PLAYER_MOTION_IDLE;
}

void player_set_input(Player *player, float x, float y) {
    float magnitude;
    if (!player) return;
    magnitude = sqrtf(x * x + y * y);
    if (magnitude <= PLAYER_INPUT_DEADZONE) {
        player->input_x = 0.0f;
        player->input_y = 0.0f;
        player->input_strength = 0.0f;
        return;
    }
    player->input_strength = clampf(magnitude, 0.0f, 1.0f);
    player->input_x = x / magnitude;
    player->input_y = y / magnitude;
}

void player_update(Player *player, uint32_t delta_ms,
                   float world_width, float world_height) {
    float speed, seconds;
    if (!player) return;

    if (player->input_strength <= PLAYER_INPUT_DEADZONE) {
        player->velocity_x = 0.0f;
        player->velocity_y = 0.0f;
        player->motion = PLAYER_MOTION_IDLE;
        return;
    }

    if (fabsf(player->input_x) > fabsf(player->input_y))
        player->facing = player->input_x < 0.0f
            ? PLAYER_FACING_LEFT : PLAYER_FACING_RIGHT;
    else
        player->facing = player->input_y < 0.0f
            ? PLAYER_FACING_UP : PLAYER_FACING_DOWN;

    player->motion = player->input_strength >= PLAYER_RUN_THRESHOLD
        ? PLAYER_MOTION_RUN : PLAYER_MOTION_WALK;
    speed = player->motion == PLAYER_MOTION_RUN
        ? player->run_speed : player->walk_speed;
    speed *= player->input_strength;

    seconds = (float)delta_ms / 1000.0f;
    player->velocity_x = player->input_x * speed;
    player->velocity_y = player->input_y * speed;
    player->x += player->velocity_x * seconds;
    player->y += player->velocity_y * seconds;

    if (world_width > PLAYER_WORLD_MARGIN * 2.0f)
        player->x = clampf(player->x, PLAYER_WORLD_MARGIN,
                           world_width - PLAYER_WORLD_MARGIN);
    if (world_height > PLAYER_WORLD_MARGIN * 2.0f)
        player->y = clampf(player->y, PLAYER_WORLD_MARGIN,
                           world_height - PLAYER_WORLD_MARGIN);
}

const char *player_animation_name(const Player *player) {
    static const char *walk[] = {
        "walk_down", "walk_up", "walk_left", "walk_left"
    };
    static const char *run[] = {
        "run_down", "run_up", "run_left", "run_left"
    };
    if (!player || player->motion == PLAYER_MOTION_IDLE) return "idle";
    if (player->facing > PLAYER_FACING_RIGHT) return "idle";
    return player->motion == PLAYER_MOTION_RUN
        ? run[player->facing] : walk[player->facing];
}

int player_animation_flip_x(const Player *player) {
    return player && player->facing == PLAYER_FACING_RIGHT &&
           player->motion != PLAYER_MOTION_IDLE;
}
