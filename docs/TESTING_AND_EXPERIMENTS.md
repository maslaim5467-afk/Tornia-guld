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

### [EXP-002] Progressive 3D Sea Rendering and Renderer Evaluation

- **Date:** 2026-11-20 (experiment recorded; execution date not yet established)
- **Subsystem / feature:** 3D renderer foundation, bounded 3D water surface, rendering quality scalability, runtime performance, and future 2D/3D renderer coexistence.
- **Branch / commit:** `feature/3d-renderer-foundation` / commit to be recorded when implementation begins.
- **Environment:** Intended Android target using the project's native Vulkan rendering path; exact device, Android version, ABI, build configuration, and graphics-driver details must be recorded when tested.
- **Objective:** Evaluate the 3D engine, which is intended to operate alongside the 2D engine, by building a sea scene in a **fixed, bounded area**. Start at the lowest practical visual and computational complexity, then increase quality in controlled stages. The experiment must measure visual correctness and runtime cost at each stage. This is an additive 3D evaluation and must not replace or regress the 2D renderer.
- **Initial scene scope:** A finite rectangular water surface with defined dimensions and a fixed initial camera/view. Do not begin with an infinite ocean, open-world water system, shoreline, or advanced environmental effects.
- **Prerequisites:**
  1. Review the current 3D math foundation and Vulkan backend integration before selecting the implementation path.
  2. Establish the smallest viable 3D scene, camera, geometry, material, and rendering path.
  3. Record the target Android device, resolution, and graphics configuration before setting quantitative performance thresholds.
  4. Keep 3D implementation and experiment changes on `feature/3d-renderer-foundation`; do not merge without review and explicit approval.
- **Procedure (progressive plan; execute one stage at a time):**
  1. **Stage 1 — Minimum quality:** Render a finite, flat, uniformly colored water plane with basic camera and depth handling. No waves, reflections, transparency, foam, shoreline, or decorative detail. Verify geometry, visibility, coordinate conventions, and stable rendering.
  2. **Stage 2 — Basic surface variation:** Add a simple, low-cost wave or surface animation. Compare its visual result and performance against Stage 1.
  3. **Stage 3 — Lighting response:** Add basic water shading and controlled light response; introduce reflections or specular highlights only if the previous stages remain stable.
  4. **Stage 4 — Higher detail:** Increase mesh/shader detail and consider foam, depth/color variation, transparency, and shoreline effects as separately measured additions. Do not add all effects at once.
  5. At every stage, record frame time/FPS, memory use, rendering stability, resolution, graphics settings, and any visual defects on the target Android device. Keep the previous stage as a baseline for comparison.
  6. After the standalone 3D stages are validated, test composition with a representative 2D scene in the same frame through the intended shared rendering architecture, without breaking the 2D path.
- **Expected result:** A finite 3D sea prototype that grows in visual complexity through measurable stages. Each stage must have a separately recorded result; no performance result or successful 2D/3D coexistence is claimed until measured and verified.
- **Observed result:** Not recorded; implementation and test execution have not yet been performed or verified.
- **Status:** Not run
- **Evidence:** To be added after each stage: screenshots or video, commit reference, build configuration, device/driver details, logs, frame-time or FPS measurements, memory observations, and later evidence of 2D/3D composition.
- **Issues and limitations:** Exact water-plane dimensions, visual reference, target device, resolution, performance thresholds, and quantitative acceptance criteria remain to be agreed. Advanced water effects are optional later-stage candidates, not initial requirements.
- **Changes made:** Experiment scope and staged test plan recorded only. No renderer implementation, visual result, benchmark, or test pass is claimed.
- **Follow-up:** Inspect the current Vulkan rendering path and define the finite test-plane dimensions and camera. Then implement Stage 1 only, build and run it, and record evidence before advancing to Stage 2.
