# ADR 0001: Web UI in JUCE's WebBrowserComponent, served from an embedded bundle

- **Status:** Accepted (spike validated on Linux/WebKitGTK; macOS verification pending)
- **Date:** 2026-11-13

## Context

Cue needs a UI for writing and browsing timestamped notes inside a DAW plugin window. Text-heavy UI (editing, lists, search, later autocomplete) is much faster to build with web tooling than with native JUCE components.

Constraints:

- **Plugin windows are child views** embedded by the host. A UI framework that wants to own the top-level window or render loop doesn't fit (this ruled out Flutter).
- **Privacy-first.** Producers under NDA are the target users, so the UI must not fetch code or data from the network at runtime.
- **The C++ layer is not a thin shell.** Parsing, validation and data ownership stay in C++, for control over security and performance.

## Decision

1. **Preact + TypeScript**, built by Vite into a static bundle (`ui/dist`).
2. **Embedded in the binary.** CMake embeds `ui/dist` as binary data (`WebUIAssets` target, `WebUIData` namespace). Vite emits stable, unhashed filenames so CMake can embed them by name.
3. **Served by a resource provider** (`WebBrowserComponent::Options::withResourceProvider`, implemented in `source/WebUIResources.cpp`). There's no localhost server, no `file://` URLs and no CDN. The provider only serves `/`, `/index.html` and `/assets/<file>` with known MIME types.
4. **Native integration** (`withNativeIntegrationEnabled` and `withNativeFunction`) for JS→C++ calls. Each function's logic lives in a plain C++ class (e.g. `NoteBridge`) that doesn't depend on the WebView and is unit tested with Catch2.
5. **Navigation is locked.** `pageAboutToLoad` only allows the resource provider root, and new-window requests are ignored. This matters because native integration exposes our functions to whatever page is loaded.
6. **One JS bridge module** (`ui/src/bridge.ts`) is the only code that talks to JUCE. In a plain browser it falls back to a mock, so the UI can be developed with Vite's dev server and no C++.
7. **Long-lived state lives in the processor**, not the editor or the page. The editor (and its WebView) is destroyed every time the plugin window closes.

## Consequences

**Good**
- Supply-chain risk for the UI is limited to build time (`npm ci` from the lockfile, `npm audit` in CI). The shipped plugin loads nothing from the network.
- UI iteration is fast (Vite with HMR and the mock bridge), and C++ logic is testable without a GUI.

**Costs and risks**
- **Three WebView backends** with different quirks: WKWebView (macOS), WebView2 (Windows), WebKitGTK (Linux).
  - **Windows:** needs WebView2 (`JUCE_USE_WIN_WEBVIEW2=1`, plus the WebView2 runtime on users' machines). Not configured yet; `WebUIResources.h` fails the build with an explicit `#error` until it is.
  - **Linux:** WebKitGTK runs in a helper process that `dlopen()`s the plugin binary. This works for the plugin but not for plain executables such as `Benchmarks`. If the helper dies, writes to its pipe raise SIGPIPE in the host process.
- **CMake requires a built UI.** Configure fails with a clear message if `ui/dist/index.html` is missing.
- **Two runtimes to keep in sync.** The JS bridge types and validation rules mirror the C++ (e.g. max note length, counted in code points on both sides).
- **JUCE's JS promises never reject.** If C++ forgets to call the completion callback, the call hangs. The bridge adds a timeout.

## Alternatives considered

- **Native JUCE components:** no WebView risk, but slow to build rich text UI, and a smaller pool of UI patterns and contributors.
- **Flutter:** conflicts with plugin windows being child views.
- **Loading from a localhost dev server or `file://` in production:** extra attack surface and fragile paths. JUCE's resource provider avoids both. (A dev-server mode for HMR *inside* the plugin may be added later, Debug builds only.)
- **WASM for the UI:** not a UI technology. It's reserved for hot paths, if profiling ever shows a bridge bottleneck.
