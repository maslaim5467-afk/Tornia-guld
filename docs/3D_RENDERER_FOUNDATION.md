# 3D renderer foundation

## Purpose

This change establishes a portable mathematical foundation for a future Vulkan
3D renderer. It does not claim that a 3D renderer, mesh upload path, depth
buffer, material system, lighting, or 3D scene is implemented yet.

## Current architecture boundary

- The existing Android/Vulkan application and its 2D drawing path remain in
  place and are not replaced by this foundation.
- Existing world, player, camera, and animation modules remain independent
  portable C modules.
- `math3d.h/.c` is a platform-independent math module. It has no Android or
  Vulkan dependency and can be reused by the 3D camera, transforms, and scene
  code.
- Matrices are column-major and use column vectors.
- `tg_mat4_look_at` uses a right-handed view convention with camera forward
  along local -Z.
- `tg_mat4_perspective_vk` uses a right-handed perspective projection with
  Vulkan normalized depth in [0, 1].

## Intended renderer separation

1. Engine/core: application lifecycle, timing, input, and scene coordination.
2. 2D renderer: existing world and sprite path, to be modularized separately.
3. 3D renderer: future mesh, camera, material, depth, and lighting path.
4. Vulkan backend: shared device, queues, swapchain, command submission, and
   GPU resource primitives. Renderer-specific pipelines and render targets
   must remain explicit rather than being mixed into gameplay code.

## Next implementation stages

1. Extract shared Vulkan device/swapchain/frame lifecycle without changing the
   current menu output.
2. Introduce a 3D camera built on the math module and test its matrices.
3. Add GPU buffer and mesh resource ownership with explicit create/destroy.
4. Add a depth attachment and a separate 3D graphics pipeline.
5. Render a minimal triangle/cube test scene, then add materials and lighting.
6. Define the frame composition contract so 2D overlays and 3D content can
   coexist without either renderer owning the other's state.

## Validation

The portable math module is tested independently with strict C11 warnings.
Android APK compilation remains a separate integration check. A passing math
test does not prove Vulkan rendering works on a physical GPU.
