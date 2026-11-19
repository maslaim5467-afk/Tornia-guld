#include "animation.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *skip_space(char *p) {
    while (*p && isspace((unsigned char)*p)) ++p;
    return p;
}

int animation_load_text(AnimationSet *set, const char *text,
                        uint16_t image_width, uint16_t image_height) {
    if (!set || !text || !image_width || !image_height) return -1;
    memset(set, 0, sizeof(*set));

    char *copy = (char *)malloc(strlen(text) + 1u);
    if (!copy) return -1;
    strcpy(copy, text);

    AnimationClip *active = NULL;
    int result = 0;
    char *save = NULL;
    for (char *line = strtok_r(copy, "\n", &save); line;
         line = strtok_r(NULL, "\n", &save)) {
        char *p = skip_space(line);
        if (!*p || *p == '#') continue;

        if (strncmp(p, "clip ", 5) == 0) {
            char name[ANIMATION_NAME_CAP], mode[8];
            unsigned duration = 0;
            if (active || set->clip_count >= ANIMATION_MAX_CLIPS ||
                sscanf(p + 5, "%31s %u %7s", name, &duration, mode) != 3 ||
                duration == 0 || duration > UINT16_MAX ||
                (strcmp(mode, "loop") != 0 && strcmp(mode, "once") != 0)) {
                result = -1; break;
            }
            for (uint16_t i = 0; i < set->clip_count; ++i) {
                if (strcmp(set->clips[i].name, name) == 0) {
                    result = -1; break;
                }
            }
            if (result) break;
            active = &set->clips[set->clip_count++];
            strcpy(active->name, name);
            active->first_frame = set->frame_count;
            active->frame_time_ms = (uint16_t)duration;
            active->loop = (uint8_t)(strcmp(mode, "loop") == 0);
        } else if (strncmp(p, "frame ", 6) == 0) {
            unsigned x, y, w, h;
            if (!active || set->frame_count >= ANIMATION_MAX_FRAMES ||
                sscanf(p + 6, "%u %u %u %u", &x, &y, &w, &h) != 4 ||
                !w || !h || x > image_width || y > image_height ||
                w > image_width - x || h > image_height - y ||
                x > UINT16_MAX || y > UINT16_MAX ||
                w > UINT16_MAX || h > UINT16_MAX) {
                result = -1; break;
            }
            AnimationFrame *frame = &set->frames[set->frame_count++];
            frame->x = (uint16_t)x; frame->y = (uint16_t)y;
            frame->width = (uint16_t)w; frame->height = (uint16_t)h;
            ++active->frame_count;
        } else if (strcmp(p, "end") == 0) {
            if (!active || !active->frame_count) { result = -1; break; }
            active = NULL;
        } else {
            result = -1; break;
        }
    }

    if (active || !set->clip_count || !set->frame_count) result = -1;
    free(copy);
    if (result) memset(set, 0, sizeof(*set));
    return result;
}

int animation_find_clip(const AnimationSet *set, const char *name) {
    if (!set || !name) return -1;
    for (uint16_t i = 0; i < set->clip_count; ++i)
        if (strcmp(set->clips[i].name, name) == 0) return (int)i;
    return -1;
}

int animation_play(AnimationPlayer *player, const AnimationSet *set,
                   const char *name) {
    if (!player || !set) return -1;
    int index = animation_find_clip(set, name);
    if (index < 0) return -1;
    player->clip_index = (uint16_t)index;
    player->frame_index = 0;
    player->elapsed_ms = 0;
    player->finished = 0;
    return 0;
}

void animation_update(AnimationPlayer *player, const AnimationSet *set,
                      uint32_t delta_ms) {
    if (!player || !set || player->clip_index >= set->clip_count) return;
    const AnimationClip *clip = &set->clips[player->clip_index];
    if (!clip->frame_count || player->finished) return;
    player->elapsed_ms += delta_ms;
    while (player->elapsed_ms >= clip->frame_time_ms) {
        player->elapsed_ms -= clip->frame_time_ms;
        if (player->frame_index + 1u < clip->frame_count) {
            ++player->frame_index;
        } else if (clip->loop) {
            player->frame_index = 0;
        } else {
            player->frame_index = (uint16_t)(clip->frame_count - 1u);
            player->elapsed_ms = 0;
            player->finished = 1;
            break;
        }
    }
}

const AnimationFrame *animation_current_frame(const AnimationPlayer *player,
                                               const AnimationSet *set) {
    if (!player || !set || player->clip_index >= set->clip_count) return NULL;
    const AnimationClip *clip = &set->clips[player->clip_index];
    if (player->frame_index >= clip->frame_count) return NULL;
    return &set->frames[clip->first_frame + player->frame_index];
}
