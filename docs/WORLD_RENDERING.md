# World and terrain renderer foundation

The experimental world now has a first on-device Vulkan scene.

## Current prototype
- A deterministic 30 x 30 grid of 128 x 128 world-unit grass cells.
- Seeded variation and sparse overlay decoration are drawn as simple colored Vulkan geometry.
- A gold locator marks the center of the test world.
- Selecting **START GAME** opens the world; dragging on the screen pans the camera.
- Camera movement is clamped to the 30 x 30 map bounds.

## Important scope note
This scene is a renderer integration milestone, not the final terrain art pass. The
grass PNG base and 12 overlay images are not sampled by Vulkan yet. Current
vegetation is a temporary geometric stand-in. The next texture pass will use
Android's native bitmap APIs and Vulkan image/sampler resources, without adding
a third-party image decoder.

## World model
Each cell has a stable variation value and zero to two ordered overlay IDs.
Overlay IDs are 1-based and map to the intended `grass_01.png` through
`grass_12.png` asset family. `world_generate` is deterministic for a given
seed and has no platform dependency.

## Host-side test
```sh
cc -std=c11 -Wall -Wextra -Werror -pedantic src/world.c tests/world_test.c -o /tmp/world_test
/tmp/world_test
```
