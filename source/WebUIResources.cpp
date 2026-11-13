#include "WebUIResources.h"
#include "WebUIData.h"

namespace
{
    const char* mimeTypeFor (const juce::String& filename)
    {
        const auto ext = filename.fromLastOccurrenceOf (".", false, false).toLowerCase();

        if (ext == "html") return "text/html";
        if (ext == "js")   return "text/javascript";
        if (ext == "css")  return "text/css";
        if (ext == "svg")  return "image/svg+xml";
        if (ext == "png")  return "image/png";
        if (ext == "json") return "application/json";
        if (ext == "woff2") return "font/woff2";

        return nullptr;
    }

    // Maps a request path to a bundle filename, or an empty string if the
    // path isn't one we serve.
    juce::String filenameForPath (juce::String path)
    {
        // Ignore query strings and fragments.
        path = path.upToFirstOccurrenceOf ("?", false, false).upToFirstOccurrenceOf ("#", false, false);

        if (path == "/" || path == "/index.html")
            return "index.html";

        const juce::String assetsPrefix { "/assets/" };
        if (!path.startsWith (assetsPrefix))
            return {};

        auto filename = path.substring (assetsPrefix.length());

        // Flat directory only: no nested paths, no "..", no backslashes.
        if (filename.isEmpty() || filename.containsAnyOf ("/\\") || filename.startsWith ("."))
            return {};

        return filename;
    }
}

std::optional<juce::WebBrowserComponent::Resource> getWebUIResource (const juce::String& path)
{
    const auto filename = filenameForPath (path);
    if (filename.isEmpty())
        return std::nullopt;

    const auto* mime = mimeTypeFor (filename);
    if (mime == nullptr)
        return std::nullopt;

    // juce_add_binary_data stores each file under a mangled identifier and
    // keeps its original (directory-less) filename alongside it.
    for (int i = 0; i < WebUIData::namedResourceListSize; ++i)
    {
        if (filename != WebUIData::originalFilenames[i])
            continue;

        int size = 0;
        const auto* data = WebUIData::getNamedResource (WebUIData::namedResourceList[i], size);
        if (data == nullptr)
            return std::nullopt;

        // reinterpret_cast: the embedded data is raw bytes typed as char;
        // Resource wants std::byte. Same memory, different type.
        const auto* bytes = reinterpret_cast<const std::byte*> (data);
        return juce::WebBrowserComponent::Resource { { bytes, bytes + size }, mime };
    }

    return std::nullopt;
}
