# 3D renderer foundation

## Purpose

This change establishes the portable math, camera, CPU mesh, GPU mesh upload,
and bounded sea-scene ownership foundations for a future Vulkan 3D renderer.
The application now creates and uploads the 20 x 20 sea test mesh after Vulkan
device/queue initialization. It does not yet draw that mesh: a 3D graphics
pipeline, depth attachment, material system, and visible 3D scene remain
unimplemented. Runtime success on a physical Android GPU is still pending.

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

## Architecture specification\n\nSee [3D_RENDERER_ARCHITECTURE.md](3D_RENDERER_ARCHITECTURE.md) for the bounded-sea Stage 1 architecture, module responsibilities, frame composition contract, resource ownership, and acceptance criteria. The specification is a plan; it does not claim these renderer modules or Vulkan features are implemented.\n\n## Intended renderer separation

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
3. **Implemented:** create the bounded 20 x 20 sea test scene and invoke the
   Vulkan GPU mesh upload after device/queue initialization. The scene owns its
   CPU arrays and opaque GPU mesh; teardown releases the GPU resource before
   destroying the device.
4. Run the APK on a physical Android device and verify the upload log/result and
   selected memory path. A successful APK build alone is not runtime proof.
5. Add a depth attachment and a separate 3D graphics pipeline, then draw the
   sea mesh from a 3D camera.
6. Define the frame composition contract so 2D overlays and 3D content can
   coexist without either renderer owning the other's state.

## Validation

The portable math module is tested independently with strict C11 warnings.
Android APK compilation remains a separate integration check. A passing math
test does not prove Vulkan rendering works on a physical GPU.
