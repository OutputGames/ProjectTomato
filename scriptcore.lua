project "TomatoScript"

    location "scriptcore/generated/"
    kind "SharedLib"
    language "C#"
    dotnetframework "4.7.2"

    targetdir "scriptcore/bin\\%{cfg.buildcfg}"

    files {
        "scriptcore/src/**.cs"
    }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        optimize "Off"
        debugdir "./"
        staticruntime "off"
        runtime "Debug"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        staticruntime "off"
        runtime "Release"