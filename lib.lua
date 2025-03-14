

local BUILD_DIR = path.join("build", _ACTION)
if _OPTIONS["cc"] ~= nil then
	BUILD_DIR = BUILD_DIR .. "_" .. _OPTIONS["cc"]
end
local BGFX_DIR = "vendor/bgfx/"
local BIMG_DIR = "vendor/bimg/"
local BX_DIR = "vendor/bx/"
local GLFW_DIR = "vendor/glfw/"
local IMGUI_DIR = "vendor/dear-imgui/"
local MINI_DIR = "vendor/miniaudio/"
local RECAST_DIR = "C:/RecastNavigation/"
local AL_DIR = "C:/Program Files (x86)/OpenAL"
local TMGL_DIR = "D:/Code/ImportantRepos/tmgl/"

local BULLET_LIBS = {
    "BulletDynamics",
    "Bullet3Collision",
    "Bullet3Common",
    "Bullet3Dynamics",
    "Bullet3Geometry",
    "LinearMath",
    "BulletInverseDynamics",
    "BulletCollision",
    "BulletDynamics"
}

function includeNavProj(proj)
    includedirs(RECAST_DIR..proj.."/Include")
    links(proj)
end

function setBxCompat()
	filter "action:vs*"
		includedirs { path.join(BX_DIR, "include/compat/msvc") }
	filter { "system:windows", "action:gmake" }
		includedirs { path.join(BX_DIR, "include/compat/mingw") }
	filter { "system:macosx" }
		includedirs { path.join(BX_DIR, "include/compat/osx") }
		buildoptions { "-x objective-c++" }

    buildoptions {
        "/Zc:__cplusplus",  -- Enable updated __cplusplus macro.
        "/Zc:preprocessor", -- Enable preprocessor conformance mode.
    }
end
    
    project "TomatoRuntime"
        kind "StaticLib"
        language "C++"
        cppdialect "C++20"
        location "generated\\"
        compileas "C++"
        targetdir "bin/%{cfg.buildcfg}"
        staticruntime "on"

        libdirs(RECAST_DIR.."Build/vs2022/lib/%{cfg.buildcfg}")

        includeNavProj("Recast")
        includeNavProj("Detour")

        buildoptions {
            "/Zc:__cplusplus",  -- Enable updated __cplusplus macro.
            "/Zc:preprocessor", -- Enable preprocessor conformance mode.
        }

        includedirs { 
            path.join(BX_DIR, "include"),
            path.join(BGFX_DIR, "include"),
            path.join(GLFW_DIR, "include"),
            path.join(TMGL_DIR, "include"),
            path.join(AL_DIR, "include"),
            path.join(MINI_DIR,"extras/miniaudio_split/"),
            "include/",
            "vendor/glm/",
            "vendor/",
            "vendor/assimp/include/",
            "vendor/bullet3/src/",
            "vendor/bimg/include/",
            "include/tomato/",
            --IMGUI_DIR,
            "vendor/enet/include/",
            "vendor/box2d/include/",
            "vendor/freetype/",
            
        }

        libdirs { "vendor/bgfx/.build/win64_vs2022/bin/", path.join(AL_DIR, "libs/Win64/"), }
    

        defines {"_CRT_SECURE_NO_WARNINGS", "GENERATOR_USE_GLM",  "TOMATO_DLLBUILD",'MONO_HOME="C:/Program Files/Mono/"', 'MSBUILD_HOME="C:/Windows/Microsoft.NET/Framework/v4.0.30319/"', 'RENDERDOC_HOME="C:/Program Files/RenderDoc"'}

        files {
            "include/**",
            "source/**",
            "resources/**",
            ".editorconfig",
            IMGUI_DIR.."**"
        }

        
        libdirs { "vendor/freetype/lib/" }
        links {"freetype", "OpenAL32"}

        removefiles { "include/testproject/**"}

        removefiles { "include/tomato/_tomato.hpp", "include/tomato/tomato.cpp" }

        links { "glfw" }
        filter "system:windows"
            links { "gdi32", "kernel32", "psapi" }
        filter "system:linux"
            links { "dl", "GL", "pthread", "X11" }
        filter "system:macosx"
            links { "QuartzCore.framework", "Metal.framework", "Cocoa.framework", "IOKit.framework", "CoreVideo.framework" }

        filter "action:vs*"
            defines{"_WINSOCK_DEPRECATED_NO_WARNINGS", "_CRT_SECURE_NO_WARNINGS"}
            dependson {"miniaudio", "tmgl"}
            links {"miniaudio.lib"}
            characterset ("MBCS")
        links {"miniaudio", "enet", "tmgl"}
        libdirs {"./bin/%{cfg.buildcfg}", "vendor/enet/lib/%{cfg.buildcfg}/", "vendor/box2d/src/%{cfg.buildcfg}/"}

        filter "configurations:Debug"
            defines { "DEBUG", "BX_CONFIG_DEBUG=1" }
            symbols "On"
            debugdir "./"
            runtime "Debug"
            optimize "Off"
            links { "bimgDebug", "bxDebug", "assimp-vc143-mtd", "box2dd" }
            libdirs {"vendor/assimp/lib/Debug/", "vendor/bullet3/lib/Debug/"}
            
            for _, lib in ipairs(BULLET_LIBS) do
                links { lib .. "_Debug" }
            end

        filter "configurations:Release"
            defines { "NDEBUG", "BX_CONFIG_DEBUG=0" }
            optimize "On"
            runtime "Release"
            links { "bimgRelease", "bxRelease", "assimp-vc143-mt", "box2d" }
            libdirs {"vendor/assimp/lib/Release/", "vendor/bullet3/lib/Release/"}
            for _, lib in ipairs(BULLET_LIBS) do
                links { lib }
            end

    project "glfw"
        kind "StaticLib"
        language "C"
        staticruntime "on"
        files
        {
            path.join(GLFW_DIR, "include/GLFW/*.h"),
            path.join(GLFW_DIR, "src/context.c"),
            path.join(GLFW_DIR, "src/egl_context.*"),
            path.join(GLFW_DIR, "src/init.c"),
            path.join(GLFW_DIR, "src/input.c"),
            path.join(GLFW_DIR, "src/internal.h"),
            path.join(GLFW_DIR, "src/monitor.c"),
            path.join(GLFW_DIR, "src/null*.*"),
            path.join(GLFW_DIR, "src/osmesa_context.*"),
            path.join(GLFW_DIR, "src/platform.c"),
            path.join(GLFW_DIR, "src/vulkan.c"),
            path.join(GLFW_DIR, "src/window.c"),
        }
        includedirs { path.join(GLFW_DIR, "include") }
        filter "system:windows"
            defines "_GLFW_WIN32"
            files
            {
                path.join(GLFW_DIR, "src/win32_*.*"),
                path.join(GLFW_DIR, "src/wgl_context.*")
            }
        filter "system:linux"
            defines "_GLFW_X11"
            files
            {
                path.join(GLFW_DIR, "src/glx_context.*"),
                path.join(GLFW_DIR, "src/linux*.*"),
                path.join(GLFW_DIR, "src/posix*.*"),
                path.join(GLFW_DIR, "src/x11*.*"),
                path.join(GLFW_DIR, "src/xkb*.*")
            }
        filter "system:macosx"
            defines "_GLFW_COCOA"
            files
            {
                path.join(GLFW_DIR, "src/cocoa_*.*"),
                path.join(GLFW_DIR, "src/posix_thread.h"),
                path.join(GLFW_DIR, "src/nsgl_context.h"),
                path.join(GLFW_DIR, "src/egl_context.h"),
                path.join(GLFW_DIR, "src/osmesa_context.h"),

                path.join(GLFW_DIR, "src/posix_thread.c"),
                path.join(GLFW_DIR, "src/nsgl_context.m"),
                path.join(GLFW_DIR, "src/egl_context.c"),
                path.join(GLFW_DIR, "src/nsgl_context.m"),
                path.join(GLFW_DIR, "src/osmesa_context.c"),                       
            }

        filter "action:vs*"
            defines "_CRT_SECURE_NO_WARNINGS"
    project "miniaudio"
        kind "StaticLib"
        language "C"
        staticruntime "on"
        targetdir "bin/%{cfg.buildcfg}"

        filter "action:vs*"
            defines "_CRT_SECURE_NO_WARNINGS"

        files {
            path.join(MINI_DIR, "extras/miniaudio_split/miniaudio.h"),
            path.join(MINI_DIR, "extras/miniaudio_split/miniaudio.c"),
        }
    include(RECAST_DIR .. "premake5.lua")
    include("vendor/enet/premake5.lua")
    include (TMGL_DIR .."lib.lua")