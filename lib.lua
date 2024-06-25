
    project "TomatoRuntime"
        kind "SharedLib"
        language "C++"
        cppdialect "C++17"
        location "generated\\"
        compileas "C++"
        targetdir "bin/%{cfg.buildcfg}"
        staticruntime "off"

        defines {"_CRT_SECURE_NO_WARNINGS", "GENERATOR_USE_GLM",  "TOMATO_DLLBUILD",'MONO_HOME="C:/Program Files/Mono/"', 'MSBUILD_HOME="C:/Windows/Microsoft.NET/Framework/v4.0.30319/"'}

        files {
            "include/**",
            "resources/**",
            ".editorconfig",
            "vendor/glm/glm/**" ,
            "vendor/sdl2/include/**", 
            "vendor/**.h",
            "vendor/generator/include/**",
            "vendor/generator/src/**",
        }
        includedirs { 
            "vendor/glfw/include/", 
            "include/",
            "include/tomato",
            "vendor/assimp/include/",
            "vendor/","vendor/imgui/" ,
            "vendor/glm",
            "vendor/sdl2/include",
            "vendor/generator/include/",
            "C:/Program Files/Mono/include/mono-2.0",
        }

        files {"vendor/imgui/backends/imgui_impl_opengl3.*", "vendor/imgui/backends/imgui_impl_glfw.*"}
        files {"vendor/imgui/*"}
        files {"vendor/imgui/misc/debuggers/**", "vendor/imgui/misc/cpp/**"}

        libdirs {"vendor/assimp/lib/Release/",  "vendor/generator/lib/%{cfg.buildcfg}/", "vendor/glew/x64",  "C:/Program Files/Mono/lib"  }

        removefiles { "include/tomatoEngine/**"}

        links {"assimp-vc143-mt.lib","mono-2.0-sgen.lib"}

        dependson {"TomatoScript"}

        links {"opengl32", "glew32", "glfw3"}
        libdirs {"vendor/glfw/lib-vc2022/"}
        includedirs {"vendor/glfw/include/"}

        --links {"vulkan-1.lib"}
        --libdirs {"C:/VulkanSDK/1.3.250.0/Lib"}
        --includedirs {"C:/VulkanSDK/1.3.250.0/Include"}

        filter "configurations:Debug"
            defines { "DEBUG" }
            symbols "On"
            debugdir "./"
            runtime "Debug"
            optimize "Off"
            --links {"SDL2d", "SDL2maind"}

        filter "configurations:Release"
            defines { "NDEBUG" }
            optimize "On"
            runtime "Release"
            --links {"SDL2", "SDL2main"}
            


        

