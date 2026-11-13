#pragma once

#include "PluginProcessor.h"

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // A WebBrowserComponent that refuses to navigate anywhere except our
    // embedded bundle. Native integration is enabled, so letting the page go
    // to an arbitrary URL would hand that site access to our native functions.
    struct SinglePageBrowser : juce::WebBrowserComponent
    {
        using WebBrowserComponent::WebBrowserComponent;
        bool pageAboutToLoad (const juce::String& newURL) override;
        void newWindowAttemptingToLoad (const juce::String&) override {}
    };

    static juce::WebBrowserComponent::Options makeBrowserOptions (PluginProcessor&);

    // Member order matters: C++ initialises members in declaration order and
    // destroys them in reverse. Keep webView last so it is destroyed first,
    // before any member its callbacks might refer to.
    SinglePageBrowser webView;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
