# 3D Renderer Architecture

## Status

- **Architecture proposal:** initial baseline for the progressive bounded-sea experiment.
- **Implementation status:** not implemented by this document.
- **Target branch:** `feature/3d-renderer-foundation`.
- **Compatibility rule:** the existing 2D path remains operational and is not replaced.

## Goals

1. Introduce a 3D rendering path that can coexist with the existing 2D path.
2. Begin with the smallest useful 3D scene: one finite, flat, uniformly colored water plane.
3. Keep platform-independent scene data separate from Android and Vulkan resource management.
4. Make GPU resource ownership and lifetime explicit.
5. Add complexity only after the preceding stage has been built, measured, and reviewed.

## Non-goals for the first sea stage

- Infinite or streamed oceans.
- Waves, water simulation, reflections, refraction, transparency, foam, shoreline, or post-processing.
- A general-purpose material editor or physics/water simulation system.
- Replacing or rewriting the existing 2D renderer.
- Claiming 2D/3D same-frame composition before it has been implemented and tested.

## Proposed module boundaries

### 1. Engine/core

Owns Android lifecycle, surface lifecycle, frame timing, input routing, and high-level scene selection. It coordinates renderers but does not contain sea geometry or water-specific shader logic.

The current `src/main.c` still owns much of this work. It is a known integration bottleneck; extraction should be incremental and must preserve the current Android startup and 2D behavior.

### 2. Shared Vulkan backend

Owns Vulkan objects whose lifetime is shared by renderers:

- VkInstance, Android surface, physical/logical device, graphics queue.
- Swapchain, swapchain images/views, frame synchronization, command pools/buffers.
- Shared frame begin/submit/present coordination.
- Common GPU allocation and resource-lifetime helpers.

The backend owns frame submission. A renderer must not independently acquire/present the swapchain or submit a competing frame.

### 3. 2D renderer

Owns 2D-specific pipelines, vertex formats, textures, sprites, tilemaps, and draw ordering. It must not depend on 3D scene types or 3D camera state.

The current 2D implementation is still substantially embedded in `src/main.c`; it is not yet a clean standalone module. Do not describe it as fully extracted.

### 4. 3D renderer

Owns 3D-specific camera state, mesh data, 3D pipelines, depth state, and 3D draw recording. Initial module candidates:

- `src/camera3d.h/.c`: camera parameters and view/projection calculation, using `math3d`.
- `src/mesh3d.h/.c`: CPU-side vertex/index descriptions and validation; GPU handles remain backend-owned or are wrapped by an explicit resource object.
- `src/renderer3d.h/.c`: 3D renderer lifecycle and recording of 3D draw commands.
- `src/sea_test_scene.h/.c`: finite test-plane dimensions, transform, and simple material parameters. This is experiment content, not a general engine subsystem.

These are proposed files, not existing implementation.

## Frame composition contract

The core coordinates one frame in this order:

1. Begin frame and acquire one swapchain image through the shared backend.
2. Begin the frame's render work.
3. Record the 3D world pass (when enabled).
4. Record the 2D world/UI pass with explicit overlay ordering.
5. End rendering, submit once, and present once through the backend.

The render target and render-pass strategy must support a depth attachment for 3D while preserving predictable 2D overlay behavior. A renderer records work into the frame coordinated by the backend; renderers do not own swapchain acquisition or presentation.

Before implementation, inspect Vulkan format/depth-format support and choose a supported depth format. Depth testing should be enabled for 3D geometry and disabled or explicitly controlled for 2D overlays. The exact render-pass/dynamic-rendering design is an implementation decision to validate against the current Vulkan setup and Android drivers.

## Initial sea scene contract — Stage 1

- **Geometry:** one finite rectangle in the XZ plane, centered at the origin, with Y = 0.
- **Initial dimensions:** 20 x 20 engine units; a test scale only, not a world-meter commitment.
- **Topology:** four vertices and six indices (two triangles).
- **Material:** one constant opaque blue color; no texture or lighting dependency.
- **Camera convention:** right-handed, Y-up, looking along local -Z, consistent with `tg_mat4_look_at`.
- **Initial camera:** position (0, 8, 12), target (0, 0, 0), up (0, 1, 0); adjust only if the plane is not framed correctly.
- **Projection:** perspective using `tg_mat4_perspective_vk`; validate Vulkan clip-space Y orientation and viewport conventions during integration.
- **No animation:** the plane is static in Stage 1.

The dimensions and camera are deterministic starting values for the prototype and may be tuned after the first rendered result. They are not visual-quality claims.

## GPU resource and lifecycle rules

- Every Vulkan resource has one documented owner and a matching create/destroy path.
- Resource creation and destruction occur only while the Vulkan device is valid; teardown waits for relevant GPU work before releasing resources.
- Handle partial initialization failures and clean up resources already created.
- Recreate swapchain-dependent resources after surface/swapchain changes.
- Avoid per-frame allocation of static mesh buffers.
- Do not expose raw Vulkan ownership to the sea scene or gameplay layer.
- Keep CPU-side geometry testable without an Android device.

## Implementation sequence

1. **Architecture and baseline review:** document module boundaries, frame contract, and the smallest sea-scene contract. No 2D behavior changes.
2. **Camera and geometry foundations:** add portable camera/mesh data and host-side tests; no Vulkan rendering claim.
3. **Backend integration design:** inspect the current monolithic Vulkan setup, select depth attachment/render-pass strategy, and define resource ownership.
4. **Minimal 3D pass:** implement a separate 3D pipeline and render the flat finite plane.
5. **Android validation:** build and run on the target device; capture evidence and record frame time, FPS, and memory.
6. **2D/3D composition:** only after the standalone 3D pass is stable, render a representative 2D layer in the same frame and verify no regression.
7. **Progressive quality stages:** add waves, lighting, and higher detail separately, preserving a benchmark baseline at each step.

## Acceptance criteria for Stage 1

- The Android build succeeds for the project's target ABI.
- A finite blue plane is visible from the defined camera.
- Projection, winding/culling, viewport orientation, and depth behavior are correct.
- No crash or validation-layer error attributable to the new 3D path is observed (when validation layers are available).
- Existing 2D path remains unchanged and continues to build; runtime regression testing is recorded separately.
- Device, Android version, GPU/driver, resolution, build configuration, FPS/frame time, and memory observations are recorded.
- No pass is declared without actual device/build evidence.

## Current limitations to resolve before implementation

- Vulkan setup, swapchain, render pass, pipeline, command recording, and presentation are currently concentrated in `src/main.c`.
- The current render pass shown in the application has a color attachment only; a 3D depth attachment has not yet been established.
- The existing vertex/pipeline setup is tailored to the current 2D drawing path.
- The existing math module supplies vector/matrix primitives, but there is no 3D camera, mesh upload path, depth pipeline, or 3D scene renderer yet.
- The build script compiles `src/*.c` and Vulkan shaders with `glslc`; new source files are included by the wildcard, while shader compilation steps must be added explicitly.

## Change control

All work stays on `feature/3d-renderer-foundation`. Keep changes scoped, run relevant host tests and Android build checks, review the diff, and do not merge into another branch until the user explicitly approves.
