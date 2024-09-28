add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", { outputdir = "./build" })

local requirements = {
    "sfml",
    "fmt",
    "qt6core",
    "boost",
}

set_runtimes("MD")
set_languages("c++20")
set_encodings("utf-8")

add_requires(requirements)
add_cxflags("/Zc:__cplusplus")

for _, file in ipairs(os.files("./*.cpp")) do
    target(path.basename(file))
        set_kind("binary")
        -- set_toolchains("clang-cl")

        add_packages(requirements)
        add_files(file)
end