TEST_CASE ("Boot performance")
{
    BENCHMARK_ADVANCED ("Processor constructor")
    (Catch::Benchmark::Chronometer meter)
    {
        std::vector<Catch::Benchmark::storage_for<PluginProcessor>> storage (size_t (meter.runs()));
        meter.measure ([&] (int i) { storage[(size_t) i].construct(); });
    };

    BENCHMARK_ADVANCED ("Processor destructor")
    (Catch::Benchmark::Chronometer meter)
    {
        std::vector<Catch::Benchmark::destructable_object<PluginProcessor>> storage (size_t (meter.runs()));
        for (auto& s : storage)
            s.construct();
        meter.measure ([&] (int i) { storage[(size_t) i].destruct(); });
    };

    // Skipped on Linux: the editor hosts a WebBrowserComponent, which on Linux
    // runs WebKit in a helper process that dlopen()s the current binary. That
    // works for the plugin (a shared library) but not for this executable, so
    // the helper exits and the editor dies with SIGPIPE. Not a plugin bug.
#if !JUCE_LINUX
    BENCHMARK_ADVANCED ("Editor open and close")
    (Catch::Benchmark::Chronometer meter)
    {
        PluginProcessor plugin;

        // due to complex construction logic of the editor, let's measure open/close together
        meter.measure ([&] (int /* i */) {
            auto editor = plugin.createEditorIfNeeded();
            plugin.editorBeingDeleted (editor);
            delete editor;
            return plugin.getActiveEditor();
        });
    };
#endif
}
