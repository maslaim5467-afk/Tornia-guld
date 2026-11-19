#ifndef TORNIA_ANIMATION_H
#define TORNIA_ANIMATION_H

#include <stdint.h>

#define ANIMATION_MAX_CLIPS 16
#define ANIMATION_MAX_FRAMES 96
#define ANIMATION_NAME_CAP 32

typedef struct {
    uint16_t x, y, width, height;
} AnimationFrame;

typedef struct {
    char name[ANIMATION_NAME_CAP];
    uint16_t first_frame;
    uint16_t frame_count;
    uint16_t frame_time_ms;
    uint8_t loop;
} AnimationClip;

typedef struct {
    AnimationFrame frames[ANIMATION_MAX_FRAMES];
    AnimationClip clips[ANIMATION_MAX_CLIPS];
    uint16_t frame_count;
    uint16_t clip_count;
} AnimationSet;

typedef struct {
    uint16_t clip_index;
    uint16_t frame_index;
    uint32_t elapsed_ms;
    uint8_t finished;
} AnimationPlayer;

int animation_load_text(AnimationSet *set, const char *text,
                        uint16_t image_width, uint16_t image_height);
int animation_find_clip(const AnimationSet *set, const char *name);
int animation_play(AnimationPlayer *player, const AnimationSet *set,
                   const char *name);
void animation_update(AnimationPlayer *player, const AnimationSet *set,
                      uint32_t delta_ms);
const AnimationFrame *animation_current_frame(const AnimationPlayer *player,
                                               const AnimationSet *set);
void animation_frame_uvs(const AnimationFrame *frame,
                        uint16_t image_width, uint16_t image_height,
                        int flip_x, float uv[8]);

#endif
