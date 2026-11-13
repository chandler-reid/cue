#pragma once

#include <juce_core/juce_core.h>

// Handles calls coming from the web UI. Deliberately has no dependency on
// WebBrowserComponent, so it can be unit tested without a GUI.
//
// Threading: JUCE always invokes native functions on the message thread, so
// this class is message-thread only. It must never be touched from
// processBlock (the audio thread).
class NoteBridge
{
public:
    // Needed because JUCE_DECLARE_NON_COPYABLE declares (deleted) constructors,
    // which suppresses the implicitly generated default constructor.
    NoteBridge() = default;

    // Max note length in Unicode code points. Mirrored in ui/src/bridge.ts.
    static constexpr int maxNoteLength = 1000;

    // JS: addNote(text: string)
    // Returns { ok: true, note: { id, text } } or { ok: false, error }.
    juce::var handleAddNote (const juce::Array<juce::var>& args);

private:
    struct Note
    {
        int id;
        juce::String text;
    };

    std::vector<Note> notes;
    int nextId = 1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoteBridge)
};
