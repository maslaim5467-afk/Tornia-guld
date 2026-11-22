# Tornia-guld — Technical Design Document (TDD)

> **Status:** Initial technical draft — Version 0.2  
> **Project:** Tornia-guld  
> **Authority:** The project creator approves game-facing behavior and final technical scope.  
> **Companion documents:** [GDD](GDD.md), [Project Overview](PROJECT_OVERVIEW.md), [Software Architecture](SOFTWARE_ARCHITECTURE.md), [Asset Specification](ASSET_SPECIFICATION.md).  
>
> **Reading rule:** This document separates verified project constraints, code observed in the integrated development snapshot, approved architectural direction, proposed implementation details, and unresolved decisions. An approved capability direction is not proof that its implementation is complete. Anything not decided remains **TBD**.

## 1. Purpose and scope

This document describes the technical requirements and implementation considerations for the Android application and the custom Tornia-guld game engine. It translates the approved design direction into engineering concerns without redefining gameplay.

**Engine graphics capability:** Tornia-guld is intended to support both 2D and 3D rendering as first-class engine capabilities. The engine will use distinct, modular 2D and 3D rendering systems coordinated through a shared graphics backend. The 3D foundation is now partially implemented: math, camera, mesh, GPU layout/resource helpers, an experimental sea scene, weather state, and ocean/sky shaders exist. Dedicated renderer modules and general 2D/3D pass coordination remain incomplete.

The initial game prototype remains focused on its 2D presentation. The engine architecture must nevertheless accommodate 3D scenes and future 2D/3D projects without requiring a separate engine or compromising the existing 2D path.

## 2. Confirmed technical baseline

| Area | Current documented direction |
|---|---|
| Initial platform | Android |
| Native application model | Android NativeActivity / Android Native App Glue |
| Primary language | C |
| Graphics API | Vulkan |
| Engine | Custom engine; no ready-made game engine is part of the current plan |
| Initial target ABI | arm64-v8a |
| Native toolchain | Android NDK and Clang |
| Build entry point | `build.sh` is documented in the project overview |
| Automated build | GitHub Actions is documented in the project overview |
| Engine graphics capability | 2D and 3D rendering are approved architectural targets |
| Initial game visual format | 2D Pixel Art |
| Initial game camera direction | 2D Top-Down |
| Input direction | Touchscreen; Virtual Joystick is documented for Level 01 |

The Android API levels, NDK version, and build-tools versions recorded in [PROJECT_OVERVIEW.md](PROJECT_OVERVIEW.md) are prototype settings, not a final device-support policy. They must be re-evaluated before release.

## 3. Current implementation snapshot

The following observations describe the integration snapshot prepared for `main`. CI and device validation are tracked separately; the presence of code does not imply release readiness or measured performance.

### 3.1 Application and rendering

- `src/main.c` currently contains Android NativeActivity integration, Vulkan initialization and rendering, swapchain-related state, input event handling, sprite texture upload, animation configuration loading, world drawing, joystick drawing, and application lifecycle handling.
- The application currently uses a central runtime state structure in `main.c`.
- The current implementation is therefore a prototype with substantial platform and rendering responsibilities concentrated in one translation unit. Further modularization is a likely engineering task; its exact timing and boundaries are TBD.
- The 2D sprite/tilemap path and the experimental 3D sea/sky path coexist in the current application code. The 3D support modules are present, but much of the Vulkan setup and frame orchestration remains in `main.c`; this is not yet a cleanly separated pair of renderer modules.

### 3.2 Player movement

- `src/player.h` and `src/player.c` define a player state containing world position, velocity, normalized input, input strength, walk/run speeds, facing, and motion state.
- The observed movement implementation applies a dead zone, selects walk/run based on input strength, updates position using frame delta time, and clamps the player to the current world bounds.
- The current movement model is not a complete collision, navigation, physics, or combat movement system.

### 3.3 World and camera

- `src/world.h` and `src/world.c` define the current prototype tile-grid data, deterministic seeded tile decoration, and camera reset/pan/clamp/follow and screen-to-world conversion functions. Grid dimensions and tile sizing remain prototype values, not final world-scale decisions.
- This grid is a current prototype foundation. It does not establish the final world dimensions, world streaming design, geographic scale, terrain authoring pipeline, or final open-world implementation.

### 3.4 Animation

- `src/animation.h` and `src/animation.c` define animation frames, named clips, looping/one-shot playback, and a per-character animation controller.
- The current limits are 16 clips per set, 96 frames per set, and 32 characters for a clip name buffer (31 visible characters plus terminator).
- The text parser accepts clip declarations with a frame duration and either `loop` or `once`, followed by frame rectangles and an `end` marker.
- The movement/animation integration note on the development branch describes the controller as reusable per character, while also explicitly stating that a multi-character entity collection, NPC AI, collision, and entity lifecycle are not implemented by that stabilization.

## 4. Engine architecture direction: modular 2D and 3D rendering

### 4.1 Architectural decision

Tornia-guld will be developed as one engine with two distinct rendering modules:

- **2D Renderer:** responsible for 2D-specific rendering features such as sprites, tilemaps, 2D cameras, 2D animation rendering, and any future 2D lighting.
- **3D Renderer:** responsible for 3D-specific rendering features such as meshes, 3D cameras, materials, depth testing, and 3D lighting.

These modules must not become two separate engines. They should share the engine core and the Vulkan graphics backend while keeping their rendering pipelines and domain-specific data structures modular.

### 4.2 Shared engine and graphics backend

The intended shared foundation includes:

- **Engine core:** lifecycle, timing, input, memory utilities, math foundations, and resource coordination.
- **Vulkan backend:** physical/logical device management, queues, command buffers, swapchain and presentation, synchronization, and shared GPU resource allocation/management.
- **Renderer coordination:** frame lifecycle and the coordination needed for 2D and 3D rendering modules to submit work through the shared backend.

The exact public APIs, ownership boundaries, synchronization strategy, and resource lifetime rules must be designed before implementation and documented as they are approved.

### 4.3 2D module preservation

- The existing 2D rendering path is the baseline that must be preserved during the architectural transition.
- Introducing 3D must not silently remove, replace, or regress existing 2D functionality.
- Any shared-backend refactor must be validated against the existing 2D prototype.
- 2D-specific features and assumptions must remain isolated from 3D-specific pipeline state where practical.

### 4.4 3D module scope

The planned 3D module is expected to provide modular foundations for:

- 3D vertex/index mesh data and mesh drawing.
- 3D camera transforms and projection.
- Depth attachment and depth testing.
- 3D material and shader/pipeline management.
- Foundational lighting.
- GPU resource handling compatible with the shared Vulkan backend.

This is the initial architectural scope, not a claim that each feature is already implemented. Advanced rendering features, physics, skeletal animation, terrain systems, and large-scale world rendering require separate design and validation.

### 4.5 Combined 2D and 3D rendering

The architecture must allow 2D and 3D content to be rendered within the same application frame when a project requires it.

The exact composition model—including render order, render targets, depth isolation, UI overlay behavior, and synchronization between passes—remains an implementation design task. It must be specified and tested rather than assumed.

### 4.6 Implementation status

| Capability | Status |
|---|---|
| Existing 2D Vulkan prototype | Present in the integrated source; automated world/player/animation tests pass, while full gameplay/device validation remains incomplete |
| Modular shared renderer/backend architecture | Shared Vulkan path exists; dedicated renderer boundaries and coordinator remain incomplete |
| Dedicated 3D renderer module | 3D support modules exist, but no complete standalone renderer3d module yet |
| 3D mesh and camera systems | Implemented as foundational modules; automated tests pass |
| Depth buffer and depth testing | Used by the current Vulkan 3D path; pipeline state still requires continued validation |
| 3D materials and lighting | Experimental ocean/sky shaders include wave shading, Fresnel/GGX response, absorption approximation and atmospheric/cloud rendering; not a general material system |
| Combined 2D/3D frame composition | Both paths exist in the application, but a general-purpose composition API and full regression/device matrix are not yet established |

## 5. Runtime responsibilities

The following responsibilities are required by the current direction. Their final module boundaries and APIs remain subject to engineering review.

| Responsibility | Purpose | Current status |
|---|---|---|
| Android platform layer | Application lifecycle, native window, asset access, touch events | Present in prototype code |
| Vulkan backend | Device/swapchain setup, command submission, shared GPU resources | Basic Vulkan path present in prototype code; modular backend architecture pending |
| 2D renderer | Sprite, tilemap, and 2D scene rendering | Basic 2D path present in prototype code |
| 3D renderer | Mesh, camera, material, depth, and lighting rendering | Foundational modules and experimental sea/sky path implemented; renderer orchestration remains in `main.c` |
| Renderer coordination | Coordinate 2D and 3D passes through the shared backend | Partial application-level coordination; general coordinator not implemented |
| Input layer | Convert touch input into game actions | Basic touch / joystick path present |
| Player movement | Update player motion and world position | Basic player module present on development branch |
| World representation | Store and generate prototype world tiles | Basic grid module present on development branch |
| Camera | Follow/pan/clamp and coordinate conversion | Basic 2D camera functions present on development branch |
| Animation playback | Select clips and advance frames | Basic animation module present on development branch |
| Collision | Resolve solid boundaries and entity/environment contacts | Not established as a complete system |
| Entity lifecycle | Create, update, and destroy multiple runtime entities | Not established as a complete system |
| NPC behavior / AI | Control non-player characters | Not established |
| Save/load | Persist game state and restore it | TBD |
| Audio | Load, mix, and play sound/music | TBD |
| Gameplay scene flow | Menus, narrative sequence, stages, transitions | Incomplete / TBD |
| Localization | Load language-specific text and fonts | Architecture and implementation details TBD |

## 6. Input and movement requirements

### 6.1 Level 01

The current [Level 01 design](LEVEL_01.md) specifies a Virtual Joystick for movement within the cage. The engineering implementation must preserve the approved player-facing behavior while leaving unspecified details open.

Requirements already implied by the design:
- Touch input must be interpreted in the correct screen coordinate space.
- Joystick input must support movement in multiple directions.
- The player must remain within the intended cage play area during the opening challenge.
- Input handling must not accidentally turn a touch intended for another UI control into movement input.

The exact joystick appearance, touch ownership rules, sensitivity, accessibility behavior, and movement tuning require further specification.

### 6.2 Separation of input and movement

The current player API already separates input assignment from movement update. Preserve that separation where practical: input devices/controllers provide intent; the movement system applies the movement rules. Do not bind gameplay movement directly to rendering code as a permanent architecture decision.

## 7. Rendering and asset handling

- Pixel Art is the approved visual direction for the initial game prototype; it does not restrict the engine's approved 3D rendering capability.
- The observed Vulkan sprite sampler uses nearest-neighbor filtering for the sprite texture path.
- The observed sprite loader decodes image data through Android bitmap APIs and uploads RGBA data to a Vulkan image.
- The current loader rejects source image dimensions above 4096 × 4096 and encoded asset files larger than 8 MiB in that code path. These are implementation guards, not approved universal asset limits.
- Sprite sheet layout, atlas packing, pivots/origins, scale policy, color-space policy, compression, and final asset naming rules remain TBD.
- 3D-specific mesh formats, material representation, texture sampling policy, shader organization, and import pipeline remain TBD and must be designed for the custom engine.

See [ASSET_SPECIFICATION.md](ASSET_SPECIFICATION.md) for the initial asset handoff contract.

## 8. Performance and device constraints

The initial target is Android arm64-v8a. Performance decisions must be based on representative target devices and measured builds rather than assumed hardware capability.

Engineering should measure at least:
- Frame time and frame pacing.
- CPU and GPU time where available.
- Memory use, including decoded textures, mesh data, and Vulkan allocations.
- Asset load/decode/upload time.
- Battery and thermal behavior during sustained play.
- Stability across pause/resume, window changes, and application lifecycle events.
- Additional GPU cost of 3D passes, depth attachments, materials, and lighting on target devices.

No numerical frame-rate, memory, battery, or minimum-device target has been approved in this document. Those values remain TBD and require creator/team agreement. The 3D renderer must be designed with Android GPU and memory constraints in mind.

## 9. Build, validation, and testing

The project overview records a native Android build pipeline and GitHub Actions. The team should keep build reproducibility and device validation visible as separate concerns.

Minimum validation categories to define:
1. Compile and package the Android application for the target ABI.
2. Install and launch on a physical Android device.
3. Verify Vulkan initialization and basic frame presentation.
4. Verify touch input and joystick behavior.
5. Verify asset loading failures are handled safely.
6. Verify pause/resume and Android lifecycle transitions.
7. Verify movement boundaries and camera behavior.
8. Verify animation parsing, clip switching, looping, and one-shot completion.
9. After the 3D foundation is implemented, verify mesh rendering, camera transforms, depth testing, and resource cleanup.
10. Verify that 2D rendering continues to work after the shared-backend changes.
11. Verify a frame containing both 2D and 3D content, including the approved composition and UI behavior.

The existence of source code or a CI workflow is not, by itself, evidence that every item above has passed. Test ownership, automation level, device matrix, and acceptance thresholds remain TBD.

## 10. Technical risks and open decisions

| Topic | Why it matters | Status |
|---|---|---|
| Module boundaries | `main.c` currently carries many responsibilities | Needs architecture review |
| Shared Vulkan backend | Must serve both renderers without coupling their pipelines | Approved direction; design pending |
| 2D regression risk | Backend refactoring could break the existing prototype | Must be guarded by regression validation |
| 3D pipeline design | Mesh, camera, depth, material, and lighting need coherent ownership | Planned; detailed design pending |
| 2D/3D composition | Render order, depth isolation, and UI overlays affect correctness | Required; detailed design pending |
| Collision model | Required for reliable world and stage interaction | TBD |
| Multi-entity runtime | Needed before multiple independently updated characters | Not established |
| World representation at scale | Current 30 × 30 grid is only a prototype | TBD |
| Asset pipeline | Current runtime loader is not a complete authoring/import pipeline | TBD |
| Save format and versioning | Needed for persistent player/world state | TBD |
| Audio subsystem | Required for a complete game experience | TBD |
| Localization pipeline | Needed for multilingual UI and content | TBD |
| Device support policy | Prototype Android settings are not release policy | TBD |
| Performance budgets | Must be measured on target devices | TBD |

## 11. Change control

- The GDD remains the authority for approved player experience and gameplay intent.
- Technical proposals must not silently change story, character facts, or gameplay rules.
- If an engineering constraint requires a design change, document the conflict and present options to the project creator before changing the GDD.
- Update this document when a technical decision is approved or implementation status materially changes.
- Mark each statement as confirmed, approved direction, proposed, implemented, tested, or TBD where that distinction affects interpretation.

## 12. Immediate engineering handoff

The next engineering review should compare the approved Level 01 requirements against the current prototype and report:
- What is already implemented in the active branch.
- What is implemented but not yet tested on a physical device.
- What is partially implemented.
- What is missing.
- Which missing items require a design decision from the creator.

The renderer architecture work should begin with a focused review of the existing Vulkan path, followed by a modularization plan that preserves the 2D renderer and introduces the 3D renderer through the shared backend. Do not infer missing gameplay rules to unblock implementation; raise them as explicit questions.
