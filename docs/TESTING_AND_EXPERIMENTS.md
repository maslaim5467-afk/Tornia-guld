# Testing and Experiments

## Purpose

This document records implementation experiments and test results for Tornia-guld. It is a technical activity log, separate from the design documents.

Only work that has actually been performed may be recorded as completed. Planned work, implementation status, and verified test results must remain distinct.

## Recording Rules

- Record the exact feature or subsystem under test.
- Describe the test environment when it affects the result.
- Separate the procedure from the observed result.
- Do not mark a test as passed unless the stated checks were actually performed.
- Distinguish host-side tests, automated CI tests, emulator tests, and tests on a physical Android device.
- Record failures and limitations as well as successful results.
- Do not treat a design decision or an implementation proposal as evidence of a successful test.
- Add dates when known. If the date or environment is unknown, mark it as unknown rather than guessing.

## Test Status

- **Not run** — the test has not been performed.
- **Passed** — all stated acceptance checks passed in the recorded environment.
- **Passed with limitations** — the main checks passed, but known limitations remain.
- **Failed** — one or more stated checks did not pass.
- **Blocked** — the test could not be completed because of a dependency or environment issue.

## Experiment Record Template

Copy this section for each new experiment.

### [EXP-000] Experiment title

- **Date:**
- **Subsystem / feature:**
- **Branch / commit:**
- **Environment:** device, Android version, ABI, build configuration, or host environment as applicable.
- **Objective:**
- **Prerequisites:**
- **Procedure:**
  1.
- **Expected result:**
- **Observed result:**
- **Status:** Not run / Passed / Passed with limitations / Failed / Blocked
- **Evidence:** test output, screenshots, logs, or relevant file references.
- **Issues and limitations:**
- **Changes made:**
- **Follow-up:**

---

## Experiment Log

### [EXP-001] 2D Pixel-Art Forest Construction

- **Date:** 2026-11-20 (recorded; execution date not yet established)
- **Subsystem / feature:** 2D world rendering, tilemap composition, sprite/asset rendering, camera presentation, and performance.
- **Branch / commit:** `feature/terrain-world-foundation` / commit not recorded here.
- **Environment:** Android target; exact device, Android version, ABI, and build configuration to be recorded when the test is run.
- **Objective:** Build a forest environment in **2D Pixel Art** using Tornia-guld. This experiment is intended to evaluate the engine's ability to render and compose a coherent forest scene from pixel-art environment assets, rather than a 3D or realistic forest.
- **Prerequisites:** Confirm the available forest/ground/vegetation pixel-art assets and their licenses; identify the current tile and sprite rendering path; ensure the Android build can be installed and launched.
- **Procedure:**
  1. Prepare a small forest test area using pixel-art ground tiles.
  2. Arrange trees and other available forest elements into a readable top-down scene, preserving consistent pixel scale and alignment.
  3. Render the scene through Tornia-guld's existing 2D rendering path.
  4. Check tile/sprite placement, layering and overlap, camera visibility, and edge behavior while viewing or moving through the test area.
  5. Record frame time, memory use, and any rendering issues on the target Android device.
- **Expected result:** A stable, visually coherent 2D top-down pixel-art forest scene rendered by Tornia-guld, with correctly aligned assets and no obvious missing textures, incorrect layering, or unintended gaps. Performance and memory observations are recorded rather than assumed.
- **Observed result:** Not recorded; the experiment has not yet been run or verified.
- **Status:** Not run
- **Evidence:** To be added after implementation and execution (screenshots, build details, logs, and measurements).
- **Issues and limitations:** Exact scene dimensions, asset set, target device, performance thresholds, and final acceptance criteria remain to be determined.
- **Changes made:** Experiment objective and initial test plan recorded only; no implementation or test result is claimed.
- **Follow-up:** Review the current forest assets and renderer, implement the smallest representative forest scene, run it on a physical Android device, and update this entry with evidence and measured results.

### [EXP-002] Detailed 3D Sea Rendering and Renderer Evaluation

- **Date:** 2026-11-20 (experiment recorded; execution date not yet established)
- **Subsystem / feature:** 3D renderer foundation, 3D scene rendering, water-surface representation, visual detail, and performance; future 2D/3D renderer coexistence.
- **Branch / commit:** `feature/3d-renderer-foundation` / commit to be recorded when implementation begins.
- **Environment:** Intended Android target using the project's native Vulkan rendering path; exact device, Android version, ABI, build configuration, and graphics-driver details must be recorded when tested.
- **Objective:** Construct a detailed 3D sea environment as a practical evaluation of the 3D engine being developed to operate alongside the existing 2D engine. The experiment must assess whether the 3D foundation can render a visually rich, convincing sea scene and provide evidence about rendering capability and runtime performance. The 3D work is additive: it must not replace or regress the 2D renderer.
- **Prerequisites:**
  1. Review the current 3D math foundation and Vulkan backend integration before selecting the implementation path.
  2. Establish a minimal 3D scene, camera, geometry, material, and rendering path.
  3. Define the target visual scope and the actual test device before setting quantitative performance thresholds.
  4. Keep 3D implementation and experiment changes on `feature/3d-renderer-foundation`; do not merge without review and explicit approval.
- **Procedure (initial plan; refine before execution):**
  1. Implement a 3D sea test scene using the engine's native rendering architecture.
  2. Develop the water surface with visible geometric and/or shader-driven detail appropriate to the selected design, such as waves, changing surface normals, and light response. The specific technique remains an implementation decision to validate.
  3. Add environmental detail needed to evaluate the sea convincingly, potentially including horizon/depth variation, reflections or specular highlights, shoreline transitions, and atmospheric lighting. These are candidate evaluation features, not yet approved mandatory scope.
  4. Provide camera movement or viewing angles that expose the sea surface, depth cues, and visual stability.
  5. Measure frame time, frame rate, memory use, and rendering stability on the target Android device; record resolution and graphics settings with the results.
  6. Verify that the 3D scene can be composed with a representative 2D scene in the same frame through the intended shared rendering architecture, without breaking the 2D path.
- **Expected result:** A detailed and visually coherent 3D sea scene rendered by Tornia-guld, with stable surface appearance and camera behavior. The experiment should produce reproducible performance observations and establish whether 3D rendering can coexist with 2D rendering in the engine architecture. No performance result or successful coexistence is claimed until measured and verified.
- **Observed result:** Not recorded; implementation and test execution have not yet been performed or verified.
- **Status:** Not run
- **Evidence:** To be added after execution: screenshots or video, commit reference, build configuration, device/driver details, logs, frame-time or FPS measurements, memory observations, and evidence of 2D/3D composition.
- **Issues and limitations:** Exact sea-scene scope, visual reference, required detail level, water technique, target device, resolution, performance thresholds, and acceptance criteria remain to be agreed. The candidate effects listed above are not commitments to implement every effect.
- **Changes made:** Experiment objective and initial test plan recorded only. No renderer implementation, visual result, benchmark, or test pass is claimed.
- **Follow-up:** Agree on the sea's visual scope and acceptance criteria; inspect the current Vulkan rendering path; then implement the smallest 3D sea prototype that can meaningfully test visual quality, performance, and coexistence with 2D.
