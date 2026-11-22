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
