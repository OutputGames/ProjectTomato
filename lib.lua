

local BUILD_DIR = path.join("build", _ACTION)
if _OPTIONS["cc"] ~= nil then
	BUILD_DIR = BUILD_DIR .. "_" .. _OPTIONS["cc"]
end
local BGFX_DIR = "vendor/bgfx/"
local BIMG_DIR = "vendor/bimg/"
local BX_DIR = "vendor/bx/"
local GLFW_DIR = "vendor/glfw/"
local IMGUI_DIR = "vendor/dear-imgui/"

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

        buildoptions {
            "/Zc:__cplusplus",  -- Enable updated __cplusplus macro.
            "/Zc:preprocessor", -- Enable preprocessor conformance mode.
        }

        includedirs { 
            path.join(BGFX_DIR, "include"),
            path.join(BX_DIR, "include"),
            path.join(GLFW_DIR, "include"),
            "include/",
            "vendor/glm/",
            "vendor/",
            "vendor/assimp/include/",
            "vendor/bullet3/src/",
            "vendor/bimg/include/",
            "include/tomato/",
            "vendor/miniaudio/",
            IMGUI_DIR,
        }

        libdirs { "vendor/bgfx/.build/win64_vs2022/bin/" }
    

        defines {"_CRT_SECURE_NO_WARNINGS", "GENERATOR_USE_GLM",  "TOMATO_DLLBUILD",'MONO_HOME="C:/Program Files/Mono/"', 'MSBUILD_HOME="C:/Windows/Microsoft.NET/Framework/v4.0.30319/"', 'RENDERDOC_HOME="C:/Program Files/RenderDoc"'}

        files {
            "include/**",
            "source/**",
            "resources/**",
            ".editorconfig",
            IMGUI_DIR.."**"
        }

        removefiles { "include/testproject/**"}

        removefiles { "include/tomato/_tomato.hpp", "include/tomato/tomato.cpp" }

        links { "glfw" }
        filter "system:windows"
            links { "gdi32", "kernel32", "psapi" }
        filter "system:linux"
            links { "dl", "GL", "pthread", "X11" }
        filter "system:macosx"
            links { "QuartzCore.framework", "Metal.framework", "Cocoa.framework", "IOKit.framework", "CoreVideo.framework" }


        filter "configurations:Debug"
            defines { "DEBUG", "BX_CONFIG_DEBUG=1" }
            symbols "On"
            debugdir "./"
            runtime "Debug"
            optimize "Off"
            links { "bgfxDebug", "bimgDebug", "bxDebug", "assimp-vc143-mtd" }
            libdirs {"vendor/assimp/lib/Debug/", "vendor/bullet3/lib/Debug/"}
            
            for _, lib in ipairs(BULLET_LIBS) do
                links { lib .. "_Debug" }
            end

        filter "configurations:Release"
            defines { "NDEBUG", "BX_CONFIG_DEBUG=0" }
            optimize "On"
            runtime "Release"
            links { "bgfxRelease", "bimgRelease", "bxRelease", "assimp-vc143-mt" }
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