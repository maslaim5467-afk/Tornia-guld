# Tornia-guld — Technical Design Document (TDD)

> **Status:** Initial technical draft — Version 0.1  
> **Project:** Tornia-guld  
> **Authority:** The project creator approves game-facing behavior and final technical scope.  
> **Companion documents:** [GDD](GDD.md), [Project Overview](PROJECT_OVERVIEW.md), [Software Architecture](SOFTWARE_ARCHITECTURE.md), [Asset Specification](ASSET_SPECIFICATION.md).  
>
> **Reading rule:** This document separates verified project constraints, code observed in the development branch, proposed technical direction, and unresolved decisions. A proposal is not an implementation commitment. Anything not decided remains **TBD**.

## 1. Purpose and scope

This document describes the technical requirements and implementation considerations for the first Android prototype and the custom game engine. It translates the approved design direction into engineering concerns without redefining gameplay.

The initial technical scope is to maintain a runnable Android application, render a 2D scene with Vulkan, receive touch input, and progressively support the first playable stage. The complete game runtime, world simulation, combat, persistence, and content pipeline are not yet fully specified.

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
| Visual format | 2D Pixel Art |
| Camera direction | 2D Top-Down |
| Input direction | Touchscreen; Virtual Joystick is documented for Level 01 |

The Android API levels, NDK version, and build-tools versions recorded in [PROJECT_OVERVIEW.md](PROJECT_OVERVIEW.md) are prototype settings, not a final device-support policy. They must be re-evaluated before release.

## 3. Current implementation snapshot

The following observations come from source files inspected on branch `feature/terrain-world-foundation`. They describe that branch snapshot and must not be interpreted as proof that the same code is already merged into `main`, release-ready, or fully tested.

### 3.1 Application and rendering

- `src/main.c` currently contains Android NativeActivity integration, Vulkan initialization and rendering, swapchain-related state, input event handling, sprite texture upload, animation configuration loading, world drawing, joystick drawing, and application lifecycle handling.
- The application currently uses a central runtime state structure in `main.c`.
- The current implementation is therefore a prototype with substantial platform and rendering responsibilities concentrated in one translation unit. Further modularization is a likely engineering task; its exact timing and boundaries are TBD.

### 3.2 Player movement

- `src/player.h` and `src/player.c` define a player state containing world position, velocity, normalized input, input strength, walk/run speeds, facing, and motion state.
- The observed movement implementation applies a dead zone, selects walk/run based on input strength, updates position using frame delta time, and clamps the player to the current world bounds.
- The current movement model is not a complete collision, navigation, physics, or combat movement system.

### 3.3 World and camera

- `src/world.h` and `src/world.c` define a 30 × 30 tile grid, a current tile-size constant of 128 world units, deterministic seeded tile decoration, and camera reset/pan/clamp/follow and screen-to-world conversion functions.
- This grid is a current prototype foundation. It does not establish the final world dimensions, world streaming design, geographic scale, terrain authoring pipeline, or final open-world implementation.

### 3.4 Animation

- `src/animation.h` and `src/animation.c` define animation frames, named clips, looping/one-shot playback, and a per-character animation controller.
- The current limits are 16 clips per set, 96 frames per set, and 32 characters for a clip name buffer (31 visible characters plus terminator).
- The text parser accepts clip declarations with a frame duration and either `loop` or `once`, followed by frame rectangles and an `end` marker.
- The movement/animation integration note on the development branch describes the controller as reusable per character, while also explicitly stating that a multi-character entity collection, NPC AI, collision, and entity lifecycle are not implemented by that stabilization.

## 4. Runtime responsibilities

The following responsibilities are required by the current direction. Their final module boundaries and APIs remain subject to engineering review.

| Responsibility | Purpose | Current status |
|---|---|---|
| Android platform layer | Application lifecycle, native window, asset access, touch events | Present in prototype code |
| Vulkan renderer | Device/swapchain setup, command submission, drawing | Present in prototype code |
| Input layer | Convert touch input into game actions | Basic touch / joystick path present |
| Player movement | Update player motion and world position | Basic player module present on development branch |
| World representation | Store and generate prototype world tiles | Basic grid module present on development branch |
| Camera | Follow/pan/clamp and coordinate conversion | Basic camera functions present on development branch |
| Animation playback | Select clips and advance frames | Basic animation module present on development branch |
| Collision | Resolve solid boundaries and entity/environment contacts | Not established as a complete system |
| Entity lifecycle | Create, update, and destroy multiple runtime entities | Not established as a complete system |
| NPC behavior / AI | Control non-player characters | Not established |
| Save/load | Persist game state and restore it | TBD |
| Audio | Load, mix, and play sound/music | TBD |
| Gameplay scene flow | Menus, narrative sequence, stages, transitions | Incomplete / TBD |
| Localization | Load language-specific text and fonts | Architecture and implementation details TBD |

## 5. Input and movement requirements

### 5.1 Level 01

The current [Level 01 design](LEVEL_01.md) specifies a Virtual Joystick for movement within the cage. The engineering implementation must preserve the approved player-facing behavior while leaving unspecified details open.

Requirements already implied by the design:
- Touch input must be interpreted in the correct screen coordinate space.
- Joystick input must support movement in multiple directions.
- The player must remain within the intended cage play area during the opening challenge.
- Input handling must not accidentally turn a touch intended for another UI control into movement input.

The exact joystick appearance, touch ownership rules, sensitivity, accessibility behavior, and movement tuning require further specification.

### 5.2 Separation of input and movement

The current player API already separates input assignment from movement update. Preserve that separation where practical: input devices/controllers provide intent; the movement system applies the movement rules. Do not bind gameplay movement directly to rendering code as a permanent architecture decision.

## 6. Rendering and asset handling

- Pixel Art is the approved visual direction.
- The observed Vulkan sprite sampler uses nearest-neighbor filtering for the sprite texture path.
- The observed sprite loader decodes image data through Android bitmap APIs and uploads RGBA data to a Vulkan image.
- The current loader rejects source image dimensions above 4096 × 4096 and encoded asset files larger than 8 MiB in that code path. These are implementation guards, not approved universal asset limits.
- Sprite sheet layout, atlas packing, pivots/origins, scale policy, color-space policy, compression, and final asset naming rules remain TBD.

See [ASSET_SPECIFICATION.md](ASSET_SPECIFICATION.md) for the initial asset handoff contract.

## 7. Performance and device constraints

The initial target is Android arm64-v8a. Performance decisions must be based on representative target devices and measured builds rather than assumed hardware capability.

Engineering should measure at least:
- Frame time and frame pacing.
- CPU and GPU time where available.
- Memory use, including decoded textures and Vulkan allocations.
- Asset load/decode/upload time.
- Battery and thermal behavior during sustained play.
- Stability across pause/resume, window changes, and application lifecycle events.

No numerical frame-rate, memory, battery, or minimum-device target has been approved in this document. Those values remain TBD and require creator/team agreement.

## 8. Build, validation, and testing

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

The existence of source code or a CI workflow is not, by itself, evidence that every item above has passed. Test ownership, automation level, device matrix, and acceptance thresholds remain TBD.

## 9. Technical risks and open decisions

| Topic | Why it matters | Status |
|---|---|---|
| Module boundaries | `main.c` currently carries many responsibilities | Needs architecture review |
| Collision model | Required for reliable world and stage interaction | TBD |
| Multi-entity runtime | Needed before multiple independently updated characters | Not established |
| World representation at scale | Current 30 × 30 grid is only a prototype | TBD |
| Asset pipeline | Current runtime loader is not a complete authoring/import pipeline | TBD |
| Save format and versioning | Needed for persistent player/world state | TBD |
| Audio subsystem | Required for a complete game experience | TBD |
| Localization pipeline | Needed for multilingual UI and content | TBD |
| Device support policy | Prototype Android settings are not release policy | TBD |
| Performance budgets | Must be measured on target devices | TBD |

## 10. Change control

- The GDD remains the authority for approved player experience and gameplay intent.
- Technical proposals must not silently change story, character facts, or gameplay rules.
- If an engineering constraint requires a design change, document the conflict and present options to the project creator before changing the GDD.
- Update this document when a technical decision is approved or implementation status materially changes.
- Mark each statement as confirmed, proposed, implemented, tested, or TBD where that distinction affects interpretation.

## 11. Immediate engineering handoff

The next engineering review should compare the approved Level 01 requirements against the current prototype and report:
- What is already implemented in the active branch.
- What is implemented but not yet tested on a physical device.
- What is partially implemented.
- What is missing.
- Which missing items require a design decision from the creator.

Do not infer missing gameplay rules to unblock implementation; raise them as explicit questions.
