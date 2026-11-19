#include "../src/animation.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
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

int main(int argc, char **argv) {
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

    {
        AnimationFrame uv_frame = {10, 20, 30, 40};
        float uv[8];
        animation_frame_uvs(&uv_frame, 100, 200, 0, uv);
        assert(uv[0] > 0.099f && uv[0] < 0.101f);
        assert(uv[2] > 0.399f && uv[2] < 0.401f);
        assert(uv[4] > 0.399f && uv[4] < 0.401f);
        assert(uv[6] > 0.099f && uv[6] < 0.101f);

        animation_frame_uvs(&uv_frame, 100, 200, 1, uv);
        assert(uv[0] > 0.399f && uv[0] < 0.401f);
        assert(uv[2] > 0.099f && uv[2] < 0.101f);
        assert(uv[4] > 0.099f && uv[4] < 0.101f);
        assert(uv[6] > 0.399f && uv[6] < 0.401f);
        assert(uv[1] == uv[3] && uv[5] == uv[7]);
    }


    assert(animation_load_text(&set, "clip bad 10 loop\nframe 30 0 8 8\nend\n", 32, 32) == -1);
    assert(set.clip_count == 0);

    if (argc > 1) {
        FILE *file = fopen(argv[1], "rb");
        assert(file != NULL);
        assert(fseek(file, 0, SEEK_END) == 0);
        long size = ftell(file);
        assert(size > 0 && size < 65536);
        rewind(file);
        char *config = (char *)malloc((size_t)size + 1u);
        assert(config != NULL);
        assert(fread(config, 1, (size_t)size, file) == (size_t)size);
        fclose(file);
        config[size] = '\0';
        assert(animation_load_text(&set, config, 1536, 1024) == 0);
        assert(set.clip_count == 10 && set.frame_count == 41);
        assert(animation_find_clip(&set, "walk_left") >= 0);
        assert(animation_find_clip(&set, "run_right") >= 0);
        free(config);
    }
    puts("animation tests passed");
    return 0;
}
