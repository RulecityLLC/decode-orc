# Copilot Instructions for Decode-Orc

## Repository Overview

**Decode-Orc** is a cross-platform orchestration framework for LaserDisc and tape decoding workflows. It provides both a GUI (`orc-gui`) and CLI (`orc-cli`) interface, sharing a common MVP-architected core (`orc/core`, `orc/presenters`, `orc/view-types`). 

- **Type:** C++ / Qt6 cross-platform application
- **Build system:** CMake 3.20+ with vcpkg dependencies
- **Reproducible builds:** Nix (recommended) with flake.nix
- **Target platforms:** Linux (Flatpak), macOS (DMG), Windows (MSI)

## Architecture & Constraints

The project enforces **MVP (Model-View-Presenter) pattern** to keep layers decoupled. Do not bypass this:

- **Model/Core:** `orc/core/` — business logic, isolated from UI
- **Presenters:** `orc/presenters/` — translates core output to view models
- **View Types:** `orc/view-types/` — shared DTO-like structures
- **View:** `orc/gui/` and `orc/cli/` — consume presenters, never touch core directly

Run `ctest -R MVPArchitectureCheck` to validate boundaries before submitting PRs.

## Licensing & Legal Requirements

**Decode-Orc is licensed under GPLv3.** All dependencies and contributions must be compatible with GPLv3:

- **Permitted licenses:** GPLv3, GPLv2, LGPL, BSD, MIT, Apache 2.0, ISC, and similar permissive licenses
- **Incompatible licenses:** AGPL (stronger copyleft), proprietary/closed-source, SSPL
- **Check before adding:** Always verify a new dependency's license in its repository or LICENSE file before proposing a PR

**For contributors:**
- When adding a new dependency (via vcpkg.json or flake.nix), document its license
- If you're unsure about license compatibility, ask in the issue or PR
- vcpkg.json and flake.nix changes will be reviewed for license compliance during CI/CD

**License file location:** See [LICENSE](../../LICENSE) in the repository root (GPLv3).

## Source Code Structure

**Repository root directory layout:**

```
decode-orc/
├── orc/                           # Main project directory (CMake target root)
│   ├── common/                    # Shared utilities (logging, file I/O, exceptions)
│   ├── core/                      # Core business logic (MVP Model layer)
│   │   ├── stages/                # Processing pipeline stages
│   │   ├── metadata/              # Metadata handling
│   │   └── [business logic]
│   ├── view-types/                # Shared DTO structures (MVP shared layer)
│   │   └── [data transfer objects used by presenters & UI]
│   ├── presenters/                # Presenter layer (MVP Presenter)
│   │   └── [translates core output to view models]
│   ├── cli/                       # Command-line interface
│   │   └── main.cpp
│   └── gui/                       # Qt6 graphical interface (optional, BUILD_GUI=ON)
│       └── [Qt widgets & dialogs]
├── orc-unit-tests/                # Unit test suite (compiled if BUILD_UNIT_TESTS=ON)
│   └── core/                      # Tests for core module
├── cmake/                         # CMake build utilities
│   ├── check_mvp_architecture.sh  # MVP boundary validation script
│   ├── MVPEnforcement.cmake       # MVP constraint macros
│   └── [other build helpers]
├── .github/                       # GitHub-specific configuration
│   ├── copilot-instructions.md    # AI agent guidance (this file)
│   ├── workflows/                 # CI/CD pipelines
│   └── ISSUE_TEMPLATE/
├── external/                      # Third-party dependencies
│   └── ld-decode-tools/           # Legacy ld-decode reference (checked in; available locally)
├── docs/                          # Documentation
├── assets/                        # Images, logos
├── CMakeLists.txt                 # Top-level CMake config
├── CMakePresets.json              # CMake build presets for all platforms
├── flake.nix                      # Nix reproducible build configuration
├── vcpkg.json                     # Dependency manifest (vcpkg)
├── BUILD.md                       # Build instructions (detailed)
├── TESTING.md                     # Testing strategy & patterns
├── CONTRIBUTING.md                # Contribution guidelines
└── README.md                      # Project overview
```

**Important dependency handling notes:**

**Nix-based workflow (recommended):**
- `flake.nix` defines all external dependencies as flake inputs
  - `qtnodes` (Qt node editor) is fetched from GitHub during `nix develop` / `nix build`
  - `ezpwd-reed-solomon` headers are fetched from GitHub during `nix develop` / `nix build`
- No git submodules required; Nix handles everything automatically
- Simply run `nix develop` and dependencies are available
- When already working inside `nix develop`, if a temporary extra tool is needed to complete the task, prefer `nix shell` to pull that tool into the active workflow instead of installing it through the host system or assuming it already exists

**Non-Nix (CMake/vcpkg) workflow:**
- Dependencies are managed via `vcpkg.json` (manifest mode)
- External dependencies may need manual setup (see BUILD.md for details)
- Git submodules are NOT used in this project's primary workflow

| Module | Purpose | Dependencies | Can depend on |
|--------|---------|--------------|---------------|
| `orc/common` | Utilities (logging, file I/O) | None | — |
| `orc/core` | Business logic; no UI knowledge | common | common |
| `orc/view-types` | DTOs for presentation layer | — | common, core (read-only output types) |
| `orc/presenters` | Translates core → view models | core, view-types | common, core, view-types |
| `orc/gui` | Qt6 UI; consumes presenters | presenters, view-types, Qt6 | presenters, view-types, common |
| `orc/cli` | CLI; consumes presenters | presenters, view-types | presenters, view-types, common |
| `orc-unit-tests` | Unit tests (mocked dependencies) | gtest | any (for testing) |

**MVP Enforcement Rules:**

- `orc/core` must **never** include GUI headers (Qt, presenters, or view-types)
- `orc/presenters` must **never** include `orc/gui` or `orc/cli` headers
- `orc/gui` and `orc/cli` must **never** directly access `orc/core` (use `orc/presenters` instead)
- Cross-layer includes are detected by `cmake/check_mvp_architecture.sh` and enforced in CI/CD

**Adding new features:**

1. **Business logic:** Add implementation to `orc/core/`; add corresponding unit tests to `orc-unit-tests/core/`
2. **Presentation layer:** Add presenter in `orc/presenters/` to translate core output; add view types to `orc/view-types/` if needed
3. **UI layer:** Add GUI dialogs/widgets to `orc/gui/` or CLI commands to `orc/cli/`; both consume presenters, never core directly

**Key configuration files:**

- **`CMakeLists.txt` (root):** Top-level project setup, MVP enforcement, subdir inclusion (`-DBUILD_UNIT_TESTS`, `-DBUILD_GUI`)
- **`orc/CMakeLists.txt`:** Main project config; defines build options; includes all subdirectories
- **`CMakePresets.json`:** Build presets for all platforms (linux-gui-debug, macos-gui-debug, windows-gui-release, etc.)
- **`flake.nix`:** Nix dev environment and reproducible build configuration
- **`vcpkg.json`:** Dependency manifest (spdlog, Qt6, FFmpeg, sqlite, yaml-cpp, fftw, gtest)

**Where to find things:**

- **Tests:** `orc-unit-tests/core/` (organized by module)
- **Headers:** `orc/<module>/` (public headers in root; internal in subdirs)
- **Build outputs:** `build/bin/` (orc-gui, orc-cli); `build/lib/` (libraries)
- **Generated files:** `build/generated/version.h` (auto-generated version info)

## Build & Test (Nix-First Recommended)

For complete build instructions including Nix setup, CMake configuration, dependency management, and troubleshooting, see [BUILD.md](../BUILD.md).

**Quick flow (Nix-based, recommended):**

```bash
# Enter Nix development environment (all dependencies, reproducible)
nix develop

# Configure with tests enabled
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_UNIT_TESTS=ON

# Build
cmake --build build -j

# Run all tests + MVP architecture check
ctest --test-dir build --output-on-failure
```

**Key build flags:**
- `BUILD_GUI=ON` (default) — build GUI; set OFF for CLI-only testing
- `BUILD_UNIT_TESTS=ON` (local) / OFF (release) — controls unit test compilation
- `CMAKE_BUILD_TYPE=Debug` (local) / Release (packaging)
- `EZPWD_INCLUDE_DIR` — path to ezpwd-reed-solomon headers (auto-set in Nix, required for manual builds)

## Testing Strategy

From TESTING.md, the project targets ~80% unit test coverage with these principles:

- **Unit tests:** Mock all dependencies; never hit filesystem, network, database, or system clock. Test one method/class in isolation.
- **Integration tests:** Sparse; reserved for happy-path wiring checks only.
- **Mocks:** Use Google Test `MOCK_METHOD` framework on interfaces (not classes).

**When adding/changing behavior:**
1. Write or update unit tests first.
2. Mock external dependencies (file I/O, network, etc.).
3. Run `ctest --test-dir build --output-on-failure` locally.
4. Verify no new MVP violations: `ctest -R MVPArchitectureCheck` or `cmake --build build --target check-mvp`.

### New Stage Expectations (Required)

The current stage portfolio has complete stage-family unit-test coverage (source, transform, sink) and shared contract coverage. New stages must preserve this standard immediately when introduced.

**Required for every new stage (`orc/core/stages/<stage_id>`):**
- Add a matching unit-test suite under `orc-unit-tests/core/stages/<stage_id>` in the same PR.
- Register/build the suite in `orc-unit-tests/core/CMakeLists.txt` using the established `gtest_discover_tests` pattern and `unit` label.
- Ensure the stage is included in stage inventory/registration checks (`StageRegistry` and node-type discovery) so contract tests continue to validate global wiring.

**Family-specific minimum behavior coverage:**
- **Source stage:**
  - descriptor schema + runtime default parity (including Edit Parameters defaults)
  - valid parameter path success + artifact metadata expectations
  - invalid parameter/dependency-failure status behavior
- **Transform stage:**
  - input precondition handling (missing/wrong/incompatible artifacts)
  - deterministic transformation behavior on controlled fixtures
  - representative parameter-branch behavior and failure propagation
- **Sink stage:**
  - output parameter schema/default parity
  - dependency interaction/status contracts using interface mocks where seams exist
  - explicit success/failure trigger status behavior without real filesystem/network/database/system-clock coupling

**Cross-cutting contract expectations for new stages:**
- Stage interface invariants remain coherent (`required_input_count`, `output_count`, node metadata vs runtime behavior).
- Parameter descriptors/defaults and runtime parsing remain consistent.
- Observation declarations, where used, remain schema/type-safe and deterministic.
- Trigger lifecycle behavior remains coherent for triggerable stages (idle/in-progress/success/failure/cancel semantics).
- New stage IDs must participate cleanly in registry and node-type discovery paths.

**Validation gates before proposing changes:**
1. `cmake --build build -j`
2. `ctest --test-dir build --output-on-failure`
3. `ctest --test-dir build -R MVPArchitectureCheck --output-on-failure`

**Review bar for stage additions:**
- Do not land a new stage implementation without its stage-family unit tests and required contract/registration coverage.
- If a contract test is intentionally skipped for a stage capability mismatch, document why in the test body so skip intent is explicit and reviewable.

## CI/CD & Multi-Platform

Current workflows (`.github/workflows/`):

- **build-and-test.yml:** Runs on Linux with Nix; executes CMake config, build, and ctest.
- **package-macos.yml:** Builds on macOS; uses Homebrew + manual vcpkg. DMG output.
- **package-flatpak.yml:** Builds on Linux; produces Flatpak bundle.
- **package-windows.yml:** Builds on Windows with MSVC 2022 & vcpkg; produces MSI.
- **release-from-artifact.yml:** Publishes artifacts to GitHub Releases (tags only).

**Before proposing changes:**
- Test locally with the Nix/Linux flow (primary CI gate).
- If modifying build system, packaging, or dependencies, review the platform-specific workflow file for that target.
- Document in PR what was validated locally and what could not be (e.g., "Unable to validate Windows MSI build on Linux"; add flag for reviewers to test).

## Contribution Hygiene

From CONTRIBUTING.md:

- **Focused changes:** One clear problem per PR.
- **Commit messages:** Clear, descriptive; reference issue numbers.
- **Documentation:** Update README.md and docs if behavior or usage changes (see BUILD.md for build-related changes).
- **Avoid refactoring unrelated code** — saves iteration and speeds review.
- **Search existing issues and PRs first** to avoid duplicates.

## Guardrails

- **Trust the instructions:** Use them as the source of truth. Search codebase only if instructions are incomplete or found to be incorrect.
- **Verify build commands:** If docs disagree with working workflows, workflows are correct; flag the discrepancy in your PR.
- **Respect architecture:** MVP layer boundaries are enforced by tests and CI.
- **Test first, refactor never:** Don't introduce new frameworks or patterns unless explicitly requested; follow existing conventions.

## Common Commands Reference

```bash
# Clean build
rm -rf build && cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_UNIT_TESTS=ON && cmake --build build -j

# Run only unit tests (skip MVP check)
ctest --test-dir build -E MVPArchitectureCheck --output-on-failure

# Check MVP architecture only
ctest --test-dir build -R MVPArchitectureCheck

# Build without tests (faster for iteration)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_UNIT_TESTS=OFF && cmake --build build -j

# Run a specific test
ctest --test-dir build -R "test_name_pattern" --output-on-failure
```
