# Tornia-guld — Software Architecture Document (SAD)

> **Status:** Initial architecture draft — Version 0.1  
> **Project:** Tornia-guld  
> **Companion documents:** [Technical Design](TECHNICAL_DESIGN.md), [Project Overview](PROJECT_OVERVIEW.md), [GDD](GDD.md).  
>
> **Important distinction:** Sections describing the current prototype are observations from the inspected `feature/terrain-world-foundation` branch. Sections describing future module boundaries are proposed architecture, not a claim that those modules already exist.

## 1. Purpose

This document records the software structure of the Tornia-guld custom engine, distinguishes the current prototype from the intended separation of responsibilities, and identifies architectural decisions that still require review.

The architecture must support the approved 2D Android game direction while remaining small enough to evolve incrementally. It must not introduce a feature, gameplay rule, or external dependency that the project creator has not approved.

## 2. Architectural constraints

- Platform: Android, initially targeting `arm64-v8a`.
- Application integration: Android NativeActivity / Native App Glue.
- Implementation language: C.
- Graphics API: Vulkan.
- Engine strategy: custom engine.
- Visual/game format: 2D Pixel Art, Top-Down camera direction.
- Initial input: touchscreen.
- The GDD is authoritative for gameplay behavior; technical architecture must implement it rather than redefine it.
- Unknown decisions remain TBD until approved.

## 3. Current prototype architecture (observed)

In the inspected development branch, the runtime is centered on `src/main.c`. It currently contains or coordinates:
- Android application callbacks and lifecycle integration.
- Vulkan instance/device/surface/swapchain and rendering state.
- Vertex submission and drawing helpers.
- Texture loading and Vulkan texture upload.
- Animation configuration loading and player animation update.
- World generation and world drawing.
- Player update and camera follow.
- Touch handling and Virtual Joystick presentation/update.
- Main-menu/prototype scene behavior.

The same branch contains separate `player`, `world`, and `animation` modules:
- `src/player.h/.c`: player movement state and update logic.
- `src/world.h/.c`: prototype tile-grid data, seeded decoration, and camera operations.
- `src/animation.h/.c`: animation metadata parsing and playback.

This is a working prototype arrangement, not yet a fully separated engine architecture. In particular, the inspected code does not establish a complete entity system, NPC framework, collision subsystem, scene manager, save system, audio engine, or general asset pipeline.

## 4. Proposed logical layers

The following is a target separation for discussion and incremental refactoring. It is not a requirement to create every module immediately.

### 4.1 Platform layer

Responsibilities:
- Android NativeActivity entry point and lifecycle callbacks.
- Native window and Android asset-manager access.
- Touch and platform input event acquisition.
- Platform-specific timing and logging.

Rule: gameplay modules should not need to call Android APIs directly. Platform-specific details should be passed through narrow interfaces where practical.

### 4.2 Core runtime layer

Responsibilities to be evaluated:
- Main loop and frame timing.
- Runtime state and application state transitions.
- Common memory, logging, and error-handling conventions.
- Configuration and feature initialization order.

Exact module boundaries and whether a separate core layer is warranted remain TBD.

### 4.3 Rendering layer

Responsibilities:
- Vulkan instance/device/surface/swapchain lifecycle.
- Render-pass/pipeline and descriptor management.
- Vertex/geometry submission.
- Texture and sprite rendering.
- Camera transforms and conversion between world and screen coordinates.

Gameplay state should determine what is drawn; rendering should not become the owner of gameplay rules.

### 4.4 Input layer

Responsibilities:
- Receive platform input events.
- Track pointer identity and touch ownership.
- Translate raw input into higher-level intent (for example, movement vector or UI activation).
- Keep input-device details separate from player movement rules.

The exact input abstraction and support for multiple simultaneous pointers are TBD.

### 4.5 World and scene layer

Responsibilities to be designed:
- Runtime world representation.
- Scene/stage lifecycle and transitions.
- World coordinate system and camera relationship.
- Interior/outdoor transitions and world loading strategy.

The GDD describes a continuous outdoor world with selected independent interiors, but streaming, persistence, loading boundaries, and world partitioning are not yet architecturally decided.

### 4.6 Entity and gameplay layer

Potential responsibilities:
- Player and other character instances.
- Character state and lifecycle.
- Movement, collision, interaction, and combat.
- NPC behavior and decision systems when designed.
- Gameplay event routing.

The existing `Player` and `AnimationController` structures are not evidence of a complete entity-component system. Whether the project should use a simple entity registry, object-oriented C structs, or an entity-component approach is TBD and should be selected based on actual needs and performance.

### 4.7 Animation layer

Responsibilities:
- Store immutable animation clip/frame metadata.
- Maintain independent playback state for each animated character.
- Select and advance clips.
- Expose the current frame rectangle and flip state to rendering.

The current animation module already follows part of this separation: an `AnimationSet` can describe shared clip/frame metadata while an `AnimationController` stores per-character playback state.

### 4.8 Asset layer

Responsibilities to be defined:
- Asset naming and directory conventions.
- Asset lookup and loading.
- Image decoding and GPU upload.
- Validation of animation configuration and image bounds.
- Lifetime, caching, and release of CPU/GPU resources.

The observed prototype loads at least some assets directly from the Android asset manager. A general-purpose asset registry/cache is not yet established.

### 4.9 Persistence layer

Responsibilities to be designed:
- Save and load player/world progress.
- Versioning and migration of save data.
- Safe writes and recovery from invalid/corrupt data.
- Storage location and Android lifecycle behavior.

No final save format or persistence architecture is approved.

### 4.10 Audio layer

Audio responsibilities, backend, formats, mixing, and lifecycle are TBD. Do not select or add a third-party audio library without the creator's approval.

## 5. Dependency direction

Proposed dependency direction:

- Platform layer provides operating-system services to the runtime.
- Input layer translates platform events into input intent.
- Gameplay systems consume intent and update game state.
- World/scene systems own stage/world state and coordinate gameplay entities.
- Animation produces frame state from gameplay animation selections.
- Rendering reads world, entity, camera, and animation state and presents it through Vulkan.
- Asset services provide validated data/resources to systems that need them.
- Persistence serializes explicitly selected game state; it should not own gameplay rules.

Avoid circular dependencies. In particular:
- Rendering should not decide gameplay outcomes.
- Input should not directly mutate arbitrary gameplay state.
- Animation playback should not own image decoding or Vulkan resource lifetime.
- Platform callbacks should not become the home for all gameplay rules.
- Save/load code should not duplicate the rules for updating game state.

These are architectural guidelines for review, not proof that the current prototype already satisfies them.

## 6. Data ownership and resource lifetime

Initial guidance:
- Per-character mutable state must be owned by that character instance.
- Shared read-only metadata may be shared when safe (the current animation-set/controller split is an example).
- GPU resources must have explicit creation and destruction paths tied to the Vulkan device lifecycle.
- Android asset handles and temporary decode buffers must not outlive their valid scope.
- Any future cache must define ownership, reference/lifetime rules, and memory limits before adoption.

A complete resource ownership policy, allocator strategy, and cache policy remain TBD.

## 7. Error handling and diagnostics

The architecture should make failures observable and recoverable where practical:
- Validate asset data before using it.
- Check platform and Vulkan return codes.
- Log failures with enough context to identify the subsystem and asset.
- Avoid continuing with partially initialized required resources.
- Define whether a failure is fatal, recoverable, or should fall back to a placeholder.

A project-wide error code convention and logging policy remain TBD.

## 8. Threading and frame execution

No multithreaded gameplay or asset-loading architecture is approved in this draft. Begin with the simplest reliable execution model and introduce worker threads only when measured performance or responsiveness requires them.

Before adding concurrency, define:
- Which data is owned by which thread.
- How asset uploads interact with Vulkan queue/device synchronization.
- How Android lifecycle events stop or resume work.
- How errors and cancellation are propagated.

## 9. Proposed runtime flow

The high-level flow below is conceptual and does not imply that each stage is already implemented:

1. Android starts the NativeActivity.
2. Platform initialization provides the native window and asset access.
3. The renderer initializes the required Vulkan resources.
4. Runtime systems load the selected scene and required assets.
5. Each frame gathers input and converts it to gameplay intent.
6. Gameplay/world systems update using frame delta time.
7. Animation playback advances from the selected character states.
8. Rendering reads the updated state and submits the frame.
9. On pause/termination, runtime work is suspended and owned resources are handled according to the lifecycle policy.

Exact fixed-step versus variable-step simulation, pause semantics, and lifecycle persistence remain TBD.

## 10. Architecture decisions still open

| Decision | Current position |
|---|---|
| Full module/file layout | TBD |
| Scene manager and scene transition model | TBD |
| Entity representation and lifecycle | TBD |
| Collision and physics model | TBD |
| NPC/AI architecture | TBD |
| World partitioning and streaming | TBD |
| Asset registry/cache | TBD |
| Save format and migration | TBD |
| Audio backend | TBD |
| Threading model | No additional threading approved |
| Memory allocation strategy | TBD |
| Test architecture | TBD |

## 11. Review and change control

The development team should review this architecture against the current code and provide:
- Existing modules and their actual responsibilities.
- Coupling or ownership problems that block the next milestone.
- The smallest safe refactor needed for the next approved feature.
- Any technical conflict with an approved GDD decision.
- Alternatives, costs, and risks for decisions that remain open.

The team may recommend architecture changes. Changes that affect gameplay behavior, narrative facts, or project constraints require creator approval before being treated as final.
