import { defineConfig } from "vite";
import preact from "@preact/preset-vite";
import { fileURLToPath } from "node:url";

// JUCE's frontend helper lives in the JUCE submodule. Aliasing it (rather than
// copying it) keeps the JS side in lockstep with the C++ JUCE version.
const juceFrontend = fileURLToPath(
  new URL("../JUCE/modules/juce_gui_extra/native/javascript/index.js", import.meta.url),
);

export default defineConfig({
  plugins: [preact()],
  resolve: {
    alias: { "juce-framework-frontend": juceFrontend },
  },
  // Relative asset URLs: the bundle is served from JUCE's resource provider
  // root, not from a web server path.
  base: "./",
  build: {
    outDir: "dist",
    emptyOutDir: true,
    rolldownOptions: {
      // Stable (unhashed) filenames: CMake embeds these into the plugin binary
      // by name. Cache-busting hashes buy nothing when the bundle ships inside
      // the plugin itself.
      output: {
        entryFileNames: "assets/[name].js",
        chunkFileNames: "assets/[name].js",
        assetFileNames: "assets/[name][extname]",
      },
      onLog(level, log, defaultHandler) {
        // JUCE's check_native_interop.js uses direct eval on an Android-only
        // code path. It's JUCE's file (not ours) and never runs on desktop.
        if (log.code === "EVAL" && log.id?.endsWith("check_native_interop.js")) return;
        defaultHandler(level, log);
      },
    },
  },
  server: {
    fs: { allow: [".."] }, // allow dev server to read the aliased JUCE file
  },
});
