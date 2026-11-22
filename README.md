# Tornia-guld

Tornia-guld is a custom Android game-engine and game project written in C with Vulkan.

The initial game direction remains a **2D top-down Pixel Art experience**. The engine repository now also contains an experimental **3D rendering foundation**, including 3D math/camera, mesh and Vulkan GPU-resource helpers, an ocean preview, procedural sky/volumetric clouds, and weather state.

## Current status

- 2D foundations: world/tile grid, player movement, camera operations, and sprite animation.
- 3D foundations: math, camera, mesh/GPU layout, Vulkan mesh resources, sea test scene, weather state, and ocean/sky shaders.
- Shared runtime: Android NativeActivity and Vulkan setup/frame orchestration are still substantially centered in `src/main.c`; dedicated renderer modules and a general 2D/3D coordinator remain future refactoring work.
- Validation: automated Android build and subsystem CI are tracked in GitHub Actions. Device performance and visual behavior must be verified separately; the Galaxy A14 camera-motion stutter remains unresolved.

## Documentation

- [Project overview](docs/PROJECT_OVERVIEW.md)
- [Game design document](docs/GDD.md)
- [Technical design](docs/TECHNICAL_DESIGN.md)
- [Software architecture](docs/SOFTWARE_ARCHITECTURE.md)
- [Engine implementation status](docs/ENGINE_IMPLEMENTATION_STATUS.md)
- [3D renderer architecture](docs/3D_RENDERER_ARCHITECTURE.md)
- [3D GPU resource design](docs/3D_GPU_RESOURCE_DESIGN.md)
- [Testing and experiments](docs/TESTING_AND_EXPERIMENTS.md)

## Build

The Android prototype build entry point is `build.sh`. See the workflows under `.github/workflows/` for the automated build and test jobs.

This repository is an active prototype, not a release-ready general-purpose engine. Implemented code, automated test results, and physical-device validation are documented separately.
