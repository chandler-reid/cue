#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#if !JUCE_WEB_BROWSER_RESOURCE_PROVIDER_AVAILABLE
    #error "Cue's UI needs WebBrowserComponent's resource provider. On Windows this requires WebView2 (JUCE_USE_WIN_WEBVIEW2=1)."
#endif

// Serves the embedded web UI bundle (ui/dist) to the WebBrowserComponent.
//
// `path` is what JUCE passes to the resource provider: "/" for the root, or
// e.g. "/assets/index.js". Only "/", "/index.html" and "/assets/<file>" are
// served, and only with a known MIME type; anything else returns nullopt.
std::optional<juce::WebBrowserComponent::Resource> getWebUIResource (const juce::String& path);
