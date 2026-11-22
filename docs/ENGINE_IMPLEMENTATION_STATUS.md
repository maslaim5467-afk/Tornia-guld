# Engine Implementation Status

> Snapshot date: 2026-11-22  
> Integration branch: `integration/2d-3d-mainline`  
> Source baseline: `experiment/sea-visible-preview` at `b2809cf`  
> This document separates implementation presence, automated tests, and physical-device verification.

## Executive status

Tornia-guld now has working prototype foundations for both 2D and 3D rendering in one Android/Vulkan application. This is not yet a fully modular, general-purpose engine: much of Vulkan lifecycle and frame orchestration remains in `src/main.c`, and a general renderer coordinator is not implemented.

The initial game direction remains 2D top-down Pixel Art. The 3D sea/sky scene is an engine capability experiment, not a change to the approved game presentation.

## 2D subsystem

| Area | Current implementation | Validation / limitation |
|---|---|---|
| World | `src/world.h/.c`; tile-grid data, seeded decoration and camera helpers | World core CI passed; final gameplay world is not implemented |
| Player | `src/player.h/.c`; movement state, walk/run selection and world-bound clamping | Player movement CI passed; collision/physics are not complete |
| Animation | `src/animation.h/.c`; clip/frame parsing and per-character playback controller | Animation CI passed; no full entity/NPC lifecycle |
| 2D rendering | Sprite/tile drawing path in the application Vulkan code | Build/test coverage exists; full 2D gameplay scene and asset pipeline remain incomplete |
| Assets | Grass tile images are preserved from `main`; player sprite directory is a placeholder | Player sprite sheets/configuration are not currently present in the integrated tree |

## 3D subsystem

| Area | Current implementation | Validation / limitation |
|---|---|---|
| Math | `src/math3d.h/.c` | Automated math tests passed |
| Camera | `src/camera3d.h/.c`, `src/sea_camera.h/.c` | Camera and sea-camera tests passed |
| Mesh | `src/mesh3d.h/.c` | Mesh tests passed |
| GPU layout/resources | `src/gpu_mesh_layout.h/.c`, `src/gpu_mesh_vk.h/.c` | Layout tests passed; device/driver validation remains separate |
| Sea scene | `src/sea_test_scene.h/.c`, `assets/shaders/sea.*` | Android APK builds; physical-device rendering was inspected by the user |
| Sky/sun/clouds | `assets/shaders/sky.*`, `assets/shaders/sun.*` | Procedural sky and volumetric cloud rendering; no full atmospheric or cloud-formation simulation |
| Weather | `src/weather.h/.c` | Weather-state tests passed; rain is not a complete particle/impact/wet-surface system |
| Performance | `assets/shaders/sky.frag` reduced cloud raymarch samples from 12 to 8 and noise octaves from 3 to 2 | User still reports camera-motion stutter; no frame-time/GPU timing data proves an improvement |

### Ocean and lighting model

The current ocean shader includes a spectral wave approximation (JONSWAP/SMB parameters), wave displacement/normal response, Fresnel/Schlick reflection, GGX-style specular response, Beer-Lambert absorption approximation, and environment/sky reflection. These are rendering models and approximations; they do not constitute a full fluid simulation.

The preview starts at 09:00 with a fixed sun direction. Date-, latitude-, and timezone-aware solar motion is not implemented.

## Shared runtime and architecture

- Android NativeActivity and Vulkan device/swapchain/frame lifecycle are present.
- 2D and 3D code paths coexist in the application prototype.
- Dedicated `renderer2d` and `renderer3d` modules are not yet cleanly separated.
- A general renderer coordinator/public pass API is not implemented.
- `src/main.c` remains a large integration unit; extraction has begun with the independent sea-camera module.
- No claim is made that a frame containing arbitrary user-defined 2D and 3D scenes is supported by a finished composition API.

## Automated validation

The source baseline `b2809cf` passed these GitHub Actions workflows:

- Android arm64-v8a APK build.
- World core tests.
- Player movement tests.
- Animation system tests.
- Weather system tests.
- 3D math, camera, mesh, GPU layout and sea-camera tests.

The integration branch must run these workflows again after documentation and integration changes. CI success verifies the automated build/tests only; it does not prove GPU frame rate, thermal behavior, or stable operation on every Android device.

## Known open work

1. Diagnose camera-motion stutter with frame-time and GPU/CPU measurements on the Galaxy A14.
2. Continue splitting `main.c` into focused modules without changing behavior.
3. Define clean 2D/3D renderer interfaces and shared Vulkan resource ownership.
4. Add player sprite assets and animation configuration before claiming a complete 2D scene.
5. Design collision/physics, scene lifecycle, NPC/entity management, save/load and audio only when their requirements are approved.
6. Complete precipitation rendering and solar/atmosphere models only when prioritized.
7. Validate on a device matrix beyond the current Galaxy A14.

## Integration safeguards

The integration snapshot is based on `main` and overlays the development source while preserving main-only documents and grass tile assets. The original feature and experiment branches remain available. The integration pull request is the review and CI gate; no direct write to `main` is performed by this preparation step.
