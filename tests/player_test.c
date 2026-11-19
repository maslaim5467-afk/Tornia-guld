#include "../src/player.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static int near(float a, float b) { return fabsf(a - b) < 0.01f; }

int main(void) {
    Player player;
    player_init(&player, 640.0f, 640.0f);
    assert(near(player.x, 640.0f) && near(player.y, 640.0f));
    assert(strcmp(player_animation_name(&player), "idle") == 0);

    player_set_input(&player, 0.0f, -1.0f);
    player_update(&player, 1000u, 3840.0f, 3840.0f);
    assert(player.motion == PLAYER_MOTION_RUN);
    assert(player.facing == PLAYER_FACING_UP);
    assert(strcmp(player_animation_name(&player), "run_up") == 0);
    assert(near(player.y, 415.0f));

    player_set_input(&player, -0.5f, 0.0f);
    player_update(&player, 1000u, 3840.0f, 3840.0f);
    assert(player.motion == PLAYER_MOTION_WALK);
    assert(player.facing == PLAYER_FACING_LEFT);
    assert(strcmp(player_animation_name(&player), "walk_left") == 0);
    assert(player.x < 640.0f);

    player_set_input(&player, 1.0f, 0.0f);
    player_update(&player, 16u, 3840.0f, 3840.0f);
    assert(player.facing == PLAYER_FACING_RIGHT);
    assert(strcmp(player_animation_name(&player), "walk_left") == 0);
    assert(player_animation_flip_x(&player) == 1);

    player_set_input(&player, 1.0f, 0.0f);
    player_update(&player, 1000u, 3840.0f, 3840.0f);
    assert(player.motion == PLAYER_MOTION_RUN);
    assert(strcmp(player_animation_name(&player), "run_left") == 0);
    assert(player_animation_flip_x(&player) == 1);

    player_set_input(&player, -1.0f, 0.0f);
    player_update(&player, 16u, 3840.0f, 3840.0f);
    assert(player.facing == PLAYER_FACING_LEFT);
    assert(strcmp(player_animation_name(&player), "run_left") == 0);
    assert(player_animation_flip_x(&player) == 0);

    player_set_input(&player, 0.0f, 0.0f);
    player_update(&player, 16u, 3840.0f, 3840.0f);
    assert(player.motion == PLAYER_MOTION_IDLE);
    assert(near(player.velocity_x, 0.0f));
    assert(strcmp(player_animation_name(&player), "idle") == 0);

    player.x = 20.0f;
    player_set_input(&player, -1.0f, 0.0f);
    player_update(&player, 1000u, 3840.0f, 3840.0f);
    assert(near(player.x, 8.0f));

    player.x = 3820.0f;
    player_set_input(&player, 1.0f, 0.0f);
    player_update(&player, 1000u, 3840.0f, 3840.0f);
    assert(near(player.x, 3832.0f));

    puts("player tests passed");
    return 0;
}
