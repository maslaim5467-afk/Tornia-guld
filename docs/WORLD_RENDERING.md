# World and terrain renderer

## Current prototype
- Deterministic 30 x 30 world with 128 x 128 world-unit cells.
- A seamless grass texture is used as the base surface on every visible cell.
- Thirteen transparent vegetation sprites are sampled from a Vulkan texture atlas and layered over the base surface.
- Overlay placement remains deterministic from the world seed, with zero to two details per cell.
- Selecting **START GAME** opens the world; dragging pans the camera, clamped to map bounds.

## Texture assets
- Base surface: `assets/tiles/grass/file_0000000016688210b7c4a280705377ef.png`.
- Overlay sprites 1–12: `grass_01.png` through `grass_12.png`.
- Overlay sprite 13: `file_0000000063808210b3fc9787a65c0cb7.png`.
- The Android platform BitmapFactory API decodes PNGs; Vulkan uploads the 14 decoded images into a 4 x 4 atlas. No third-party image decoder is added.
- Atlas cells are normalized to 256 x 256 pixels at load time. The GPU texture is 1024 x 1024 RGBA8.

## Rendering
Each world cell samples atlas cell 0. Its overlay IDs sample cells 1–13 and use alpha blending, preserving transparent edges. The atlas uses nearest-neighbor sampling for the pixel-art assets.

## Host-side test
```sh
cc -std=c11 -Wall -Wextra -Werror -pedantic src/world.c tests/world_test.c -o /tmp/world_test
/tmp/world_test
```
