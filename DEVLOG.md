# Cue Dev Log

Newest entries first. Each entry covers status, what was verified, and next steps.

---

## 2026-11-13: WebView ↔ C++ round-trip spike

**Goal:** Validate the riskiest assumption: a Preact UI in JUCE's `WebBrowserComponent`, served from an embedded bundle, calling into C++ and getting a result back. See [ADR 0001](docs/adr/0001-web-ui-in-webbrowsercomponent.md).

### Status

- **UI:** Vite + Preact + TypeScript in `ui/`, four exact-pinned packages. `ui/src/bridge.ts` is the single JUCE boundary, with a mock for plain browsers.
- **C++:**
  - `NoteBridge` implements `addNote(text)` with input validation.
  - `WebUIResources` serves the embedded bundle.
  - `PluginEditor` hosts the WebView with navigation locked to the bundle.
  - `PluginProcessor` owns the `NoteBridge`, so it outlives the editor.
- **Build:**
  - `JUCE_WEB_BROWSER=1`, and `ui/dist` is embedded via `juce_add_binary_data`.
  - CI builds the UI (`npm ci`, `npm audit`, `npm run build`) before configuring.
- **Dev environment:** `.devcontainer/` added (JUCE Linux deps, WebKitGTK, Node 24, headless GUI tools).

### Verified

- [x] Unit tests: `NoteBridge` validation and `WebUIResources` path handling, including traversal attempts. All ctest targets pass, with zero compiler warnings.
- [x] UI in a plain browser (`npm run dev`) runs against the mock bridge.
- [x] **End-to-end on Linux (WebKitGTK)** in the codespace, run headless under Xvfb + openbox and driven with `xdotool`:
  - The page shows "Backend: juce".
  - Adding a note returns `#1` from C++.
  - An empty submit shows C++'s "Note is empty" error.
- [ ] **End-to-end on macOS (WKWebView)**: next step, see below.

### How to verify on macOS

```bash
git submodule update --init --recursive
(cd ui && npm ci && npm run build)
cmake -B Builds -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build Builds
open "Builds/Cue_artefacts/Debug/Standalone/Pamplejuce Demo.app"
```

**Pass:**
- The UI shows **Backend: juce**.
- Adding a note lists it as `#1: …`.
- Submitting an empty note shows **Note is empty**.

Also run `ctest --test-dir Builds --output-on-failure`. On macOS the "Editor open and close" benchmark creates real WKWebViews, which is a useful smoke test of repeated editor open/close.

### Lessons and gotchas

- **JUCE's JS helper** (`index.js`) reads `window.__JUCE__` at module load, so it must be imported dynamically, never statically.
- **JUCE's native-function promises never reject.** The bridge adds a timeout.
- **`JUCE_DECLARE_NON_COPYABLE` suppresses the implicit default constructor.** Declare `Foo() = default;`.
- **Linux, NEEDS_WEB_BROWSER:** this only configures the plugin target. Tests and Benchmarks also need `juce::pkgconfig_JUCE_BROWSER_LINUX_DEPS` and `juce_link_with_embedded_linux_subprocess()`.
- **Linux, WebKit helper:** the helper `dlopen()`s the current binary, which fails for executables. Editor-creating code can't run in `Tests` or `Benchmarks` on Linux (it dies with SIGPIPE).
- **Headless GUI testing on Linux** needs a window manager (openbox). Without one, mouse and keyboard input never reach the WebKit window.
- **Vite 8 (rolldown)** reports warnings via `onLog`, not `onwarn`.

### Next steps

1. **macOS verification** of the round-trip (above).
2. **Load existing notes on open.** Add a `getNotes()` native function, so the UI rebuilds itself from C++ when the editor reopens.
3. **Playhead timestamps.** Read `getPlayHead()` in `processBlock` into atomics, have the message thread poll them, and send them to JS with `emitEventIfBrowserIsVisible`.
4. **Persistence.** Store notes in `getStateInformation` / `setStateInformation` (inside the DAW project, never uploaded). Needs an ADR. These can be called off the message thread, so `NoteBridge` will need protection.
5. **Enable VST3/AU** and test in a DAW (Reaper, Logic) plus pluginval.

### Open items / tech debt

- **Placeholder identity:** `PRODUCT_NAME`, `COMPANY_NAME`, `BUNDLE_ID` and the plugin codes are still the Pamplejuce defaults.
- **LICENSE:** still Pamplejuce's MIT license; it needs to become AGPL before going public.
- **CSP:** add a Content-Security-Policy to `ui/index.html` once it can be tested on every backend.
- **Windows:** WebView2 setup (see ADR 0001).
- **Linux robustness:** if the WebKit helper dies, SIGPIPE can kill the host. Investigate before supporting Linux.
- **Optional:** Debug-only dev-server mode, so Vite HMR runs inside the plugin window.
