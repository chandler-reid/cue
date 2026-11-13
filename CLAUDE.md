# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## About This Project

This project is derived from the [Pamplejuce](https://github.com/sudara/pamplejuce) template — a JUCE audio plugin template using CMake, C++23, and modern CI/CD. It builds cross-platform (macOS, Windows, Linux) with support for multiple plugin formats (VST3, AU, AUv3, CLAP, Standalone).

The template provides the build system, CI/CD, and project structure. The plugin-specific logic lives in `source/`.

## Build Commands

```bash
# Build the web UI first: CMake embeds ui/dist and fails to configure without it.
# Re-run after UI changes; the next cmake --build re-embeds the bundle.
(cd ui && npm ci && npm run build)

# UI-only development against a mock backend (no C++ needed)
(cd ui && npm run dev)

# Configure (run once, or after CMakeLists.txt changes)
cmake -B Builds -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build Builds --config Release

# Run tests (from project root)
ctest --test-dir Builds --verbose --output-on-failure

# Or run tests directly
./Builds/Tests

# Run a single test by name
./Builds/Tests "[test name]"

# Run benchmarks
./Builds/Benchmarks
```

For faster builds, add Ninja: `cmake -B Builds -G Ninja -DCMAKE_BUILD_TYPE=Release`

On macOS for universal binary: `-DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"`

## Project Structure

- `source/` - Plugin source code (PluginProcessor, PluginEditor, NoteBridge, WebUIResources)
- `ui/` - Preact + TypeScript web UI (Vite). `ui/src/bridge.ts` is the only module that talks to JUCE
- `docs/adr/` - Architecture Decision Records
- `DEVLOG.md` - Status, verification and next steps
- `tests/` - Catch2 test files
- `benchmarks/` - Catch2 benchmark files
- `cmake/` - CMake modules (Tests.cmake, Benchmarks.cmake, Assets.cmake, etc.)
- `modules/` - Git submodules: clap-juce-extensions, melatonin_inspector
- `JUCE/` - JUCE framework (git submodule)
- `assets/` - Binary resources (auto-included via juce_add_binary_data)
- `packaging/` - Installer resources and scripts

## Architecture

**SharedCode Library**: The `SharedCode` INTERFACE library links plugin source code to both the main plugin target and the Tests target, avoiding ODR violations.

**CMake Modules**:
- `PamplejuceVersion.cmake` - Reads VERSION file, optional auto-bump patch level
- `Assets.cmake` - Auto-includes all files in assets/ as binary data
- `Tests.cmake` - Configures Catch2 test target
- `Benchmarks.cmake` - Configures Catch2 benchmark target
- `PamplejuceIPP.cmake` - Intel IPP integration (optional)

**Test Discovery**: Uses `catch_discover_tests()` with `PRE_TEST` discovery mode for Xcode compatibility.

## Key Configuration

Edit `CMakeLists.txt` to customize:
- `PROJECT_NAME` - Internal name (no spaces)
- `PRODUCT_NAME` - Display name in DAWs (can have spaces)
- `COMPANY_NAME` - Used for bundle name
- `BUNDLE_ID` - macOS bundle identifier
- `FORMATS` - Plugin formats to build (Standalone AU VST3 AUv3)
- `PLUGIN_MANUFACTURER_CODE` / `PLUGIN_CODE` - 4-character plugin IDs

Version is read from the `VERSION` file in project root.

## Code Quality

Always resolve any compile warnings encountered during builds. Warnings should be treated as errors and fixed before considering a task complete.

Note: LSP/clangd often reports false positive diagnostic errors (like "undeclared identifier", "file not found") because it doesn't have full context of the JUCE module system. Ignore these unless the actual build fails.

## Includes

JUCE modules include common standard library headers (`<vector>`, `<algorithm>`, `<string>`, `<memory>`, etc.) so you don't need to add those explicitly in JUCE code. Adding them is harmless but redundant.

## Realtime Safety

For anything in the audio thread / hot DSP path (e.g. `processBlock`):
- Allocate in constructors or `prepareToPlay`, not while rendering audio
- Avoid dynamic allocations and container growth (`std::vector::push_back`, map insertion, string building)
- Prefer fixed-size storage (`std::array`, preallocated buffers, fixed-capacity queues)
- Keep operations deterministic and lock-free where possible

## Code Style

Uses `.clang-format`

--- 

The following is a handoff document from conversations using the Claude web application. 

# Cue: Handoff

## What it is
Cue (formerly DAWNotes) is a privacy-first, timestamped note-taking plugin for DAWs.
It is built solo, as a personal pain point from music production and a possible side business.
It is a long-term "garden" project with no commercial deadline.

- **License/model:** AGPL plugin, with paid cloud sync as the revenue moat (Bitwarden/Ardour model).
- **Hard constraint:** producers under NDA can't upload project files, so privacy-first architecture is a requirement, not a preference.

## Architecture
- **Plugin:** C++ with JUCE (migrated from iPlug2). VST3/AU, targeting macOS, Windows and Linux.
- **UI:** Preact in JUCE's `WebBrowserComponent`, loaded from a static bundle.
  - This takes runtime CDN compromise out of the threat model; supply chain risk is build-time only.
  - Needs `JUCE_WEB_BROWSER` enabled.
- **Parsing/logic:** mostly in C++ for control over security and performance. The C++ layer is deliberately *not* a thin shell.
- **Backend:** Go on Cloud Run (GCP). Firestore is the front-runner for cloud storage, SQLite likely for local.
- **Later:** React Native companion app, DAW project file parsing (standalone daemon), privacy-preserving autocomplete.
- **Repos:** public monorepo for plugin + JS UI (`chandler-reid/cue`), private repo for backend/infra.

## Decisions and principles
- **Validate the seams first.** Prove the riskiest integrations with minimal spikes before committing to the full design.
- **The audio thread is a hard boundary.** No mutexes, allocation or I/O. Use lock-free structures (e.g. `AbstractFifo`). This ruled out Go for the plugin.
- **Get foundational security right early.** Operational and marketing infrastructure can wait for real users.
- **Minimize third-party dependencies.**
- **Test and measure before adding complexity.** Use telemetry and synthetic load testing before delta sync or optimistic updates.
- **mTLS scope:** internal service-to-service and authenticated sync APIs only. Do not use it on the public update channel; use code signing (GCP Cloud KMS) there.
- **Feature flags:** simple and temporary, with no versioned flag namespaces.

## Rejected or scoped out
- **Flutter:** DAWs embed plugin windows as child views, which conflicts with Flutter owning the top-level render loop.
- **WASM:** not a UI alternative. Add it only if profiling shows a concrete bridge bottleneck.
- **devcontainers:** they solve environment reproducibility, not cross-platform behavior. Windows/macOS testing still needs native CI runners.

## Open or deferred
- **Database choice (cloud and local):** blocked on how much the backend should know about project structure versus user-authored notes.
- **Code signing via Cloud KMS:** deferred to the distribution phase (Authenticode via Jsign).
- **React Native app and WASM:** deferred pending POC validation and profiling.
- **Trademark registration:** before public launch.

## Current state (verify against repo)
- Pamplejuce-based plugin builds and loads in AudioPluginHost.
- CMake monorepo and devcontainer are set up.
- clang-tidy runs as an advisory CI job (issue #6).
- **Next step:** the spike for the `WebBrowserComponent` + Preact + C++ message round-trip. This is the top unvalidated assumption.

## Toolchain
- **Build:** CMake with `CMakePresets.json` (and `CMakeUserPresets.json` for local overrides), Ninja, `compile_commands.json` for clangd.
- **Editor:** Neovim (clangd, vtsls, treesitter) or VS Code with clangd, CMake Tools and CodeLLDB.
- **CI:** GitHub Actions and pluginval.
- **Linting:** clang-tidy, golangci-lint + gosec, Checkov/tfsec.
- **JS:** `npm ci`, pinned exact versions, `npm audit` as a CI gate, Turborepo.
- **Formatting:** `.clang-format` (Google style, 80 columns) and `.editorconfig`.
- **C++ reading:** Vital and Surge for audio, SerenityOS and fmtlib for modern C++ patterns.

## Working agreement (for Claude Code)
- The developer is new to C++. Explain C++ concepts, best practices and pitfalls as they come up.
- Produce strong documentation: ADRs for significant decisions, and `DEVLOG.md` for status and next steps.
- GitHub Issues for tracking. Contributions: bug fixes welcome, unsolicited features not.