# Stage Test Expectations

## Purpose

This document captures the permanent test expectations for stage work in `orc/core/stages`.
It is intended to replace planning-only notes and provide a stable definition of done for stage additions and major stage behavior changes.

## Scope

Applies to:
- New stage implementations.
- Existing stage changes that affect parameters, behavior, metadata, status, or dependency interactions.

Does not apply to:
- Pure refactors with no behavior change (unless test seams or contracts change).

## Required Per-Stage Deliverables

For every stage added under `orc/core/stages/<stage_id>`:
- Add a matching test suite under `orc-unit-tests/core/stages/<stage_id>` in the same PR.
- Register tests in `orc-unit-tests/core/CMakeLists.txt` using `gtest_discover_tests` with label `unit` and the appropriate family label (`sources`, `transforms`, `sinks`, or `contracts`).
- Ensure the stage participates in registry/discovery contract coverage (`StageRegistry`, node-type discovery, and project-to-DAG paths where applicable).

## Stage-Family Minimum Behavior Coverage

### Source stages

Required coverage:
- Parameter descriptor schema is correct (names, types, options/ranges where relevant).
- Descriptor defaults match runtime defaults and Edit Parameters defaults.
- Valid parameter path succeeds and produces expected source artifact metadata.
- Invalid parameter values and dependency/factory failures produce coherent failure status.

### Transform stages

Required coverage:
- Parameter descriptor/default parity (descriptor, runtime, and Edit Parameters).
- Input preconditions are enforced (missing/wrong/incompatible artifacts).
- Deterministic transformation behavior on controlled fixtures.
- Representative parameter-branch behavior.
- Error/failure propagation produces coherent status.

### Sink stages

Required coverage:
- Output parameter descriptor/default parity.
- Dependency interaction/status contracts through interface mocks where seams exist.
- Explicit trigger success and failure status behavior.
- No filesystem/network/database/system-clock coupling in unit tests.

## Shared Contract Expectations

New and changed stages must continue to satisfy shared contract coverage for applicable capabilities:
- Stage interface invariants remain coherent (`required_input_count`, `output_count`, metadata coherence).
- Parameter schema/defaults remain consistent with runtime parsing.
- Observation declarations and runtime reads/writes remain schema- and type-safe.
- Trigger lifecycle remains coherent for triggerable stages (idle, in-progress, success, failure, cancel).
- Artifact/provenance behavior is deterministic and consistent with stage intent.
- Registry/factory and node-type discovery paths remain valid.
- Project-to-DAG conversion and validation remain valid for representative happy paths.

## Skips and Capability Mismatches

When a contract test is intentionally skipped because a stage capability is not applicable:
- Keep the skip explicit in the test body.
- Document why the capability does not apply.
- Ensure neighboring required contracts for that stage are still covered.

## Validation Gates (Pre-PR)

Run all of the following before proposing stage changes:

```bash
cmake --build build -j
ctest --test-dir build --output-on-failure
ctest --test-dir build -R MVPArchitectureCheck --output-on-failure
```

## Review Checklist

A stage change is not ready to merge unless all are true:
- Stage test suite exists and is wired into CMake.
- Family-specific minimum behavior coverage is present.
- Relevant shared contract tests pass.
- No MVP boundary regressions.
- Any intentional skips are documented with stage-capability rationale.
