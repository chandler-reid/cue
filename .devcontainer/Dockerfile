# Cue development container (Linux). Used by GitHub Codespaces and VS Code Dev Containers.
# Keep the JUCE deps in sync with .github/workflows/build_and_test.yml.
FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-24.04

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        # Build tooling
        ninja-build clangd gdb \
        # JUCE Linux dependencies
        libasound2-dev libx11-dev libxinerama-dev libxext-dev libxrandr-dev libxcursor-dev \
        libfreetype6-dev libfontconfig1-dev libglu1-mesa-dev \
        # WebBrowserComponent (JUCE_WEB_BROWSER=1)
        libgtk-3-dev libwebkit2gtk-4.1-dev \
        # Headless GUI testing: virtual display, window manager (needed for input
        # focus to reach the WebView), input automation and screenshots
        xvfb openbox xdotool imagemagick \
    && rm -rf /var/lib/apt/lists/*
