
    project "TomatoEngine"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++17"
        location "generated\\"
        compileas "C++"
        targetdir "bin/%{cfg.buildcfg}"

        defines {"_CRT_SECURE_NO_WARNINGS", "GENERATOR_USE_GLM"}

        files {
            "include/**",
            "source/**",
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
            }
        
            files {"vendor/imgui/backends/imgui_impl_opengl3.*"}
        files {"vendor/imgui/backends/imgui_impl_sdl2.*", "vendor/imgui/*"}
        files {"vendor/imgui/misc/debuggers/**", "vendor/imgui/misc/cpp/**"}
            libdirs {"vendor/assimp/lib/Release/", "vendor/sdl2/lib", "vendor/generator/lib/%{cfg.buildcfg}/", "vendor/glew/x64" }

            links {"assimp-vc143-mt.lib"}

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
            links {"SDL2d", "SDL2maind"}

        filter "configurations:Release"
            defines { "NDEBUG" }
            optimize "On"
            links {"SDL2", "SDL2main"}


        

