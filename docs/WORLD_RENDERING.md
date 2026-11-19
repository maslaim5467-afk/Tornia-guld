# World and terrain renderer foundation

This change introduces a renderer-independent 30 x 30 world model. It does not
replace the current menu renderer or claim to display the terrain textures yet.

## World data

- Each cell is one 128 x 128 world-unit grass tile.
- A cell has a stable variation value and zero to two ordered overlay IDs.
- Overlay IDs are 1-based and map to `grass_01.png` through `grass_12.png`.
- `world_generate` uses a coordinate hash and a caller-provided seed. It has no
  runtime RNG state, heap allocation, or platform dependency, so a given seed
  always produces the same map.
- The base grass texture is a renderer responsibility and is not duplicated in
  every cell.

## Camera

The camera stores world-space top-left coordinates and a zoom factor. Pan input
is converted from screen-space pixels into world-space displacement and then
clamped against the 30 x 30 world bounds. When the viewport is larger than the
world at the selected zoom, the world is centered on that axis.

## Next renderer stage

The Vulkan texture path remains a separate step: native Android image decoding,
staging-buffer upload, sampled VkImage resources, descriptor sets, UV vertices,
and alpha blending. Overlay order in the world data is the draw order. No
third-party image decoder is introduced by this foundation.

## Host-side test

Run:

```sh
cc -std=c11 -Wall -Wextra -Werror src/world.c tests/world_test.c -o /tmp/world_test
/tmp/world_test
```
