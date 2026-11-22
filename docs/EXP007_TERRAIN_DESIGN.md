# EXP-007 — Procedural Rolling Terrain Foundation

## Approved baseline
- Terrain: rolling, forested hills; medium scale.
- World: 512 m × 512 m; 1 m sample spacing; 512 × 512 quads.
- Logical chunk target: 64 m × 64 m (8 × 8). Actual chunk splitting is deferred.
- Initial height scale: 24 m; static geometry only.
- No trees, vegetation, water, erosion, LOD, or runtime deformation in this first slice.

## Mathematical model
The generator uses deterministic seeded value noise with quintic interpolation across five fBm octaves. Base frequency is 0.0032 cycles/metre, lacunarity 2.0, gain 0.5. A restrained ridged component (weight 0.22) is blended with fBm. This is a phenomenological heightfield, not a physical erosion simulation.

For H(x,z), central-difference derivatives are used:
- dH/dx = [H(x+e,z) - H(x-e,z)] / (2e)
- dH/dz = [H(x,z+e) - H(x,z-e)] / (2e)
- N = normalize(-dH/dx, 1, -dH/dz)

## Implementation
- src/terrain_generator.h/.c implements seeded height sampling, normal sampling, mesh generation, ownership cleanup, and height statistics.
- TgTerrain owns position-only vertices and uint32 indices; its TgMesh3D is a non-owning view.
- Position-only layout is intentional: current TgMeshVertex3D and Vulkan upload path copy only position. Adding normals/UVs must be coordinated with vertex input and shaders.
- Mesh indices use upward-facing winding and are validated through tg_mesh3d_init.
- build.sh compiles src/*.c, so the module is included in Android native builds.
- Terrain is not yet submitted to the frame renderer. The current renderer has a sea-specific pipeline, not a general terrain draw interface.

## Baseline data
The default configuration contains 263,169 vertices and 1,572,864 indices (524,288 triangles). Position storage is approximately 3.0 MiB and index storage approximately 6.0 MiB, before GPU copies and allocator overhead.

## Validation and outstanding work
The dedicated C test checks deterministic output, mesh dimensions and bounds, non-flat relief, upward winding, normalized normals, invalid configuration, and cleanup. CI and Android APK build must pass. Physical-device rendering, generation time, peak RAM, upload cost, and frame time remain unverified.

## Next
1. Run CI and Android build.
2. Define a dedicated terrain vertex input/shader/pipeline or a safe minimal position-only terrain pass.
3. Integrate a bounded terrain scene without changing sea/sky behavior.
4. Measure on Galaxy A14 before adding chunk splitting, erosion, culling, or LOD.

## Vulkan preview integration (second implementation slice)

- Added `src/terrain_test_scene.h/.c`: generates the deterministic baseline and uploads its mesh through the existing Vulkan mesh uploader. CPU terrain data and the GPU mesh are owned by the scene object.
- Added dedicated `terrain.vert` and `terrain.frag` shaders. The vertex stage transforms the position-only mesh; the fragment stage derives a face normal from screen-space position derivatives and applies simple height tint plus diffuse lighting. This avoids changing the shared mesh vertex layout or the sea shader contract.
- Added a dedicated terrain graphics pipeline and a separate menu entry, **TERRAIN TEST**. The existing construction/ocean screen remains a separate scene.
- The terrain preview uses the existing 3D camera gesture path (drag to orbit; two-finger gesture translates/zooms through the camera helper).
- Vulkan terrain resources are released after device idle during shutdown.
- `build.sh` now compiles both terrain shaders into SPIR-V before the Android native build.

## Current scope and caveats

- The full 512 × 512 heightfield is currently one GPU mesh, not 64 uploaded chunks. The 8 × 8 / 64 m chunk grid remains the next management experiment.
- No frustum culling, LOD, streaming, erosion, water, or vegetation is implemented.
- Height-derived fragment normals are a preview technique. The next quality step is a dedicated terrain vertex format with precomputed normals and UVs, coordinated with an explicit terrain shader interface.
- Automated C tests passed on the terrain implementation branch. The Android APK workflow for the complete Vulkan preview is still pending at the time of this update; physical-device rendering and performance are not yet verified.
