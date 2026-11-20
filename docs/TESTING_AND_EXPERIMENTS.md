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

No experiment records have been added yet. Entries will be added as implementation and testing work is selected and performed.
