#ifndef TORNIA_PLAYER_H
#define TORNIA_PLAYER_H

#include <stdint.h>

typedef enum {
    PLAYER_FACING_DOWN = 0,
    PLAYER_FACING_UP,
    PLAYER_FACING_LEFT,
    PLAYER_FACING_RIGHT
} PlayerFacing;

typedef enum {
    PLAYER_MOTION_IDLE = 0,
    PLAYER_MOTION_WALK,
    PLAYER_MOTION_RUN
} PlayerMotion;

/* World-space foot position; input is supplied independently by a controller. */
typedef struct {
    float x, y;
    float velocity_x, velocity_y;
    float input_x, input_y, input_strength;
    float walk_speed, run_speed;
    PlayerFacing facing;
    PlayerMotion motion;
} Player;

void player_init(Player *player, float world_x, float world_y);
void player_set_input(Player *player, float x, float y);
void player_update(Player *player, uint32_t delta_ms,
                   float world_width, float world_height);
const char *player_animation_name(const Player *player);

#endif
