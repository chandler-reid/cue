#include "NoteBridge.h"

namespace
{
    juce::var makeError (const juce::String& message)
    {
        // DynamicObject is reference counted. Wrapping it in a var hands
        // ownership to the var, so there is no manual delete anywhere.
        juce::DynamicObject::Ptr result = new juce::DynamicObject();
        result->setProperty ("ok", false);
        result->setProperty ("error", message);
        return juce::var (result.get());
    }
}

juce::var NoteBridge::handleAddNote (const juce::Array<juce::var>& args)
{
    // Treat everything from the WebView as untrusted input.
    if (args.size() != 1 || !args[0].isString())
        return makeError ("addNote expects a single string argument");

    const auto text = args[0].toString().trim();

    if (text.isEmpty())
        return makeError ("Note is empty");

    if (text.length() > maxNoteLength)
        return makeError ("Note is too long");

    const auto id = nextId++;
    notes.push_back ({ id, text });

    juce::DynamicObject::Ptr note = new juce::DynamicObject();
    note->setProperty ("id", id);
    note->setProperty ("text", text);

    juce::DynamicObject::Ptr result = new juce::DynamicObject();
    result->setProperty ("ok", true);
    result->setProperty ("note", juce::var (note.get()));
    return juce::var (result.get());
}
