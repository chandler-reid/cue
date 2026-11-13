#include <NoteBridge.h>
#include <catch2/catch_test_macros.hpp>

namespace
{
    juce::var call (NoteBridge& bridge, juce::Array<juce::var> args)
    {
        return bridge.handleAddNote (args);
    }
}

TEST_CASE ("addNote accepts a valid note", "[NoteBridge]")
{
    NoteBridge bridge;
    const auto result = call (bridge, { "  first idea  " });

    REQUIRE (result["ok"] == juce::var (true));
    CHECK (result["note"]["id"] == juce::var (1));
    CHECK (result["note"]["text"] == juce::var ("first idea")); // trimmed
}

TEST_CASE ("addNote assigns increasing ids", "[NoteBridge]")
{
    NoteBridge bridge;
    call (bridge, { "a" });
    const auto second = call (bridge, { "b" });

    CHECK (second["note"]["id"] == juce::var (2));
}

TEST_CASE ("addNote rejects bad input", "[NoteBridge]")
{
    NoteBridge bridge;

    SECTION ("empty or whitespace-only")
    {
        const auto result = call (bridge, { "   " });
        CHECK (result["ok"] == juce::var (false));
        CHECK (result["error"] == juce::var ("Note is empty"));
    }

    SECTION ("too long")
    {
        const auto result = call (bridge, { juce::String::repeatedString ("x", NoteBridge::maxNoteLength + 1) });
        CHECK (result["ok"] == juce::var (false));
        CHECK (result["error"] == juce::var ("Note is too long"));
    }

    SECTION ("exactly max length is allowed")
    {
        const auto result = call (bridge, { juce::String::repeatedString ("x", NoteBridge::maxNoteLength) });
        CHECK (result["ok"] == juce::var (true));
    }

    SECTION ("not a string")
    {
        const auto result = call (bridge, { 42 });
        CHECK (result["ok"] == juce::var (false));
    }

    SECTION ("wrong number of arguments")
    {
        CHECK (call (bridge, {})["ok"] == juce::var (false));
        CHECK (call (bridge, { "a", "b" })["ok"] == juce::var (false));
    }

    SECTION ("failed calls don't consume ids")
    {
        call (bridge, { "" });
        CHECK (call (bridge, { "ok" })["note"]["id"] == juce::var (1));
    }
}
