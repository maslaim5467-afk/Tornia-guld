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


### [EXP-003] Android Startup Crash During Main-Menu Graphics Pipeline Creation

- **Date:** 2026-11-22
- **Subsystem / feature:** Vulkan main-menu graphics pipeline creation and Android startup stability.
- **Branch / commit:** `experiment/sea-visible-preview` / fix commit `da7075f9` (short SHA recorded from the implementation session).
- **Environment:** Physical Samsung Galaxy A14, Android 15, ARM64. The user confirmed that the updated build launched and the previously observed startup crash no longer occurred. Exact build identifier and GPU-driver version were not recorded.
- **Objective:** Diagnose and correct a repeatable native startup crash observed during Vulkan graphics-pipeline creation.
- **Original failure evidence:**
  - Multiple native crashes were recorded on 2026-11-21 (approximately 17:19, 17:31, 17:37:42, and 17:37:53).
  - The crash signal was `SIGSEGV / SEGV_MAPERR`, with fault address `0x14`.
  - The repeated stack included `libGLES_mali.so` and `libgame.so (setup+3164)`.
  - The matching CI APK was symbolized; `setup+3164` mapped to the call to `vkCreateGraphicsPipelines` for the main-menu pipeline.
  - The menu render pass included a depth/stencil attachment, while the menu pipeline creation structure did not provide a valid `pDepthStencilState` pointer. This violated the applicable Vulkan valid-usage requirement (VUID `VkGraphicsPipelineCreateInfo-renderPass-09028`).
  - The depth target creation messages appeared before the crash, so depth-target allocation was not identified as the direct failing call in this incident.
- **Changes made:** Added an explicit `VkPipelineDepthStencilStateCreateInfo` for the menu pipeline, with depth testing and depth writes disabled, and supplied it through `VkGraphicsPipelineCreateInfo.pDepthStencilState`. Diagnostic output was also added around pipeline creation. The fix is recorded in commit `da7075f9`.
- **Procedure:**
  1. Analyze the native crash log and identify the repeated crashing program counter.
  2. Match the `setup+3164` offset against the symbols and disassembly from the CI-built ARM64 APK.
  3. Inspect the corresponding menu-pipeline creation code and compare its state with Vulkan valid-usage requirements.
  4. Add an explicit depth/stencil state to the menu pipeline and build a new APK.
  5. Install and launch the updated APK on the physical Galaxy A14.
- **Expected result:** The main-menu graphics pipeline is created without the invalid null depth/stencil-state pointer, and the previously reproducible startup crash does not occur.
- **Observed result:** The user confirmed on 2026-11-22 that the updated build launched on the Galaxy A14 and that the previously observed crash was no longer occurring. This is a successful initial device verification of the reported failure path.
- **Status:** Passed with limitations
- **Evidence:** Original crash log `/mnt/data/tornia_crash.log`; symbol/disassembly analysis of CI artifact from workflow run `44907664068`; source correction in commit `da7075f9`; user-confirmed successful launch on the physical device. A new post-fix crash log and extended stability-test results have not yet been recorded.
- **Issues and limitations:**
  - A similar crash can still occur if another graphics pipeline or Vulkan resource path contains a separate invalid state, synchronization error, lifetime error, or device/driver-specific issue.
  - The exact original failure should not be considered permanently impossible: a regression that removes or bypasses the explicit depth/stencil state could reintroduce it.
  - One successful launch confirms the reported startup path passed once; it does not establish long-duration stability or prove that all Vulkan valid-usage requirements are satisfied.
  - Repeated cold starts, extended runtime, scene transitions, and fresh log inspection remain outstanding.
- **Follow-up:**
  1. Perform multiple cold launches on the Galaxy A14 and record the number of successful launches.
  2. Keep the application running and exercise the sea preview and other rendering paths; record any new crash or Vulkan error.
  3. Capture a fresh log after testing and verify that no new native crash occurs and that the menu-pipeline success diagnostic is present.
  4. If a crash recurs, compare its signal, fault address, stack, and symbolized program counter with the original `setup+3164` incident before assuming it is the same defect.
  5. Preserve this entry as the incident record and append subsequent verification results rather than rewriting the original observation.


### [EXP-004] 3D Sky, Volumetric Clouds and Weather Foundation

- **Date:** 2026-11-22
- **Subsystem:** BUILDING sky, volumetric clouds, shared wind, ocean waves and precipitation preview.
- **Implemented:** Camera-oriented Vulkan sky pass; procedural volumetric cloud raymarch (12 samples); fixed 09:00 preview sun shared by sky/sun/ocean; weather state for time, wind speed/direction, cloud coverage/altitude and rain rate; wind drives cloud drift and existing JONSWAP/SMB ocean-wave parameters; dry default; basic precipitation streak preview; 2D sky fallback if sky pipeline creation fails; weather-state unit test and CI workflow.
- **Not implemented:** Date/latitude-aware solar movement, dynamic meteorological wind fields, cloud formation physics, particle rain, wet-surface accumulation and physically based rain/ocean coupling.
- **Automated CI:** Passed on commit `a7729c489be077a3bbb4ce631bf301ed91a7165a`: Android arm64-v8a APK build and upload (run `45008961548`), weather-state tests (run `45008961623`), plus the existing world, 3D math/camera/mesh, player, and animation suites.
- **Device verification:** Android runtime and physical-device visual tests have not yet been performed.
- **Status:** Passed with limitations (automated build and unit tests only)
- **Limitations:** Clouds are procedural volumetric rendering, not fluid simulation. The 09:00 sun is a fixed preview because date/location/timezone are not modeled. Rain is a shader placeholder, not particle precipitation.
- **Follow-up:** Compile shaders/build APK; inspect on Galaxy A14; profile GPU cost; implement particle rain and astronomical sun movement later.
