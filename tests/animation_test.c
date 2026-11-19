#include "../src/animation.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char sample[] =
    "clip idle 100 loop\n"
    "frame 0 0 16 16\n"
    "frame 16 0 16 16\n"
    "end\n"
    "clip hit 50 once\n"
    "frame 0 16 8 8\n"
    "frame 8 16 8 8\n"
    "end\n";

int main(void) {
    AnimationSet set;
    AnimationPlayer player = {0};
    assert(animation_load_text(&set, sample, 32, 32) == 0);
    assert(set.clip_count == 2 && set.frame_count == 4);
    assert(animation_find_clip(&set, "idle") == 0);
    assert(animation_find_clip(&set, "missing") == -1);

    assert(animation_play(&player, &set, "idle") == 0);
    animation_update(&player, &set, 100);
    assert(player.frame_index == 1);
    animation_update(&player, &set, 100);
    assert(player.frame_index == 0);

    assert(animation_play(&player, &set, "hit") == 0);
    animation_update(&player, &set, 100);
    assert(player.frame_index == 1 && player.finished);
    const AnimationFrame *frame = animation_current_frame(&player, &set);
    assert(frame && frame->x == 8 && frame->y == 16);

    assert(animation_load_text(&set, "clip bad 10 loop\nframe 30 0 8 8\nend\n", 32, 32) == -1);
    assert(set.clip_count == 0);
    puts("animation tests passed");
    return 0;
}
