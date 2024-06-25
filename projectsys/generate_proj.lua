workspace "_PROJECTNAME"

    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }

    filter { "platforms:x32" }
        system "Windows"
        architecture "x86"

    filter { "platforms:x64" }
        system "Windows"
        architecture "x64"

    project "_PROJECTNAME"

        location "generated/"
        kind "ConsoleApp"
        language "C#"
        dotnetframework "4.7.2"

        targetdir "Bin\\%{cfg.buildcfg}"

        files {
            "**.cs"
        }

        links {
            "TomatoScript"
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