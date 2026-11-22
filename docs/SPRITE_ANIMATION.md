# Sprite animation system

## Current prototype

- The player uses one source PNG: `assets/sprites/characters/player/file_00000000ce0c8210854d0157a262ddc5.png`.
- `player.anim` stores named clips and source rectangles. Rectangles are pixel coordinates measured from the top-left of the source image.
- The engine decodes the complete PNG once with Android `BitmapFactory`, preserves its original dimensions and alpha channel, and uploads it once as a Vulkan texture.
- Each rendered frame changes only the sampled UV rectangle. The source image is not split into separate PNG files and is not uploaded again during animation.
- Frame timing is driven by elapsed monotonic time, not by assuming a fixed display refresh rate.
- Pixel-art sampling uses nearest-neighbor filtering.
- The renderer samples the current frame at the player's world-space position. The player movement controller selects idle, directional walk and directional run clips from the same metadata file.

## Metadata format

```text
clip <name> <frame-duration-ms> <loop|once>
frame <x> <y> <width> <height>
...
end
```

The loader validates every rectangle against the decoded source image dimensions. Invalid metadata rejects the animation set instead of allowing out-of-bounds texture coordinates.

## Supported clips in the current sheet

- `idle`
- `walk_down`, `walk_up`, `walk_left`, `walk_right`
- `run_down`, `run_up`, `run_left`, `run_right`
- `actions` (one-shot)

## Current limits

- Metadata rectangles are authored explicitly; automatic computer-vision detection of arbitrary sprite sheets is not part of this prototype.
- The player movement controller currently drives idle, walk and run clip selection through the reusable per-character controller. The renderer applies the mirror flag to the UVs for right-facing movement; the pure UV mapping is covered by host tests. The sheet also contains separately labelled right-facing rows, but the current movement controller intentionally uses the verified left-facing frames mirrored until the right-facing source frames are confirmed visually. Action clips are registered but are not yet triggered by gameplay events.
- The sprite texture is currently a dedicated Vulkan sampled image; a general asset/resource manager is still future work. Characters using different sprite sheets will need their own loaded `AnimationSet`/texture resource, while each character still keeps independent playback state.

## Host tests

```sh
cc -std=c11 -Wall -Wextra -Werror -pedantic src/animation.c tests/animation_test.c -o /tmp/animation_test
/tmp/animation_test
```
