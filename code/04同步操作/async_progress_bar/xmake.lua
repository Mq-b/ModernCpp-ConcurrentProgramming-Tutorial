add_rules("mode.debug", "mode.release")

set_runtimes("MD")

target("async_progress_bar")
    add_rules("qt.widgetapp","qt.quickapp")
    set_languages("c++17")
    add_files("./*.cpp")
    add_files("./*.ui")
    add_files("./*.h")
    add_headerfiles("./*.h")
target_end()