#include "PluginEditor.h"
#include "WebUIResources.h"

juce::WebBrowserComponent::Options PluginEditor::makeBrowserOptions (PluginProcessor& processor)
{
    // Capturing `processor` by reference is safe: the processor always
    // outlives its editor, and the editor owns the WebView.
    return juce::WebBrowserComponent::Options {}
        .withNativeIntegrationEnabled()
        .withResourceProvider ([] (const juce::String& path) { return getWebUIResource (path); })
        .withNativeFunction ("addNote",
            [&processor] (const juce::Array<juce::var>& args,
                juce::WebBrowserComponent::NativeFunctionCompletion complete) {
                // Called on the message thread. Always complete: the JS
                // promise has no other way to settle.
                complete (processor.getNoteBridge().handleAddNote (args));
            });
}

bool PluginEditor::SinglePageBrowser::pageAboutToLoad (const juce::String& newURL)
{
    return newURL.startsWith (getResourceProviderRoot());
}

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p),
      webView (makeBrowserOptions (p))
{
    addAndMakeVisible (webView);
    webView.goToURL (juce::WebBrowserComponent::getResourceProviderRoot());

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (480, 360);
}

PluginEditor::~PluginEditor() = default;

void PluginEditor::paint (juce::Graphics& g)
{
    // Shown only until the WebView paints over it.
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void PluginEditor::resized()
{
    webView.setBounds (getLocalBounds());
}
