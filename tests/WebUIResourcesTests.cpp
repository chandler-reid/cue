#include <WebUIResources.h>
#include <catch2/catch_test_macros.hpp>

namespace
{
    juce::String bodyOf (const juce::WebBrowserComponent::Resource& r)
    {
        return juce::String::fromUTF8 (reinterpret_cast<const char*> (r.data.data()), (int) r.data.size());
    }
}

TEST_CASE ("Web UI root serves index.html", "[WebUIResources]")
{
    const auto resource = getWebUIResource ("/");

    REQUIRE (resource.has_value());
    CHECK (resource->mimeType == "text/html");
    CHECK (bodyOf (*resource).contains ("<div id=\"app\">"));
    CHECK (getWebUIResource ("/index.html").has_value());
    CHECK (getWebUIResource ("/?foo=bar").has_value());
}

TEST_CASE ("Web UI serves the JS entry point", "[WebUIResources]")
{
    const auto resource = getWebUIResource ("/assets/index.js");

    REQUIRE (resource.has_value());
    CHECK (resource->mimeType == "text/javascript");
    CHECK_FALSE (resource->data.empty());
}

TEST_CASE ("Web UI rejects paths outside the bundle", "[WebUIResources]")
{
    CHECK_FALSE (getWebUIResource ("/does-not-exist.js").has_value());
    CHECK_FALSE (getWebUIResource ("/assets/missing.js").has_value());
    CHECK_FALSE (getWebUIResource ("/assets/../index.html").has_value());
    CHECK_FALSE (getWebUIResource ("/assets/sub/index.js").has_value());
    CHECK_FALSE (getWebUIResource ("/assets/..\\index.html").has_value());
    CHECK_FALSE (getWebUIResource ("/assets/").has_value());
    CHECK_FALSE (getWebUIResource ("").has_value());
}
