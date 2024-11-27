 workspace "TomatoEngine"
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }
 
    filter { "platforms:x32" }
        system "Windows"
        architecture "x86"
    
    filter { "platforms:x64" }
        system "Windows"
        architecture "x64"

         
    flags { "MultiProcessorCompile"}

    filter "configurations:Debug"
        defines { "DEBUG", "BX_CONFIG_DEBUG=1" }
        symbols "On"
        debugdir "./"
        runtime "Debug"
        optimize "Off"
        
        --links {"SDL2d", "SDL2maind"}

    filter "configurations:Release"
        defines { "NDEBUG", "BX_CONFIG_DEBUG=0" }
        optimize "On"
        runtime "Release"
        --links {"SDL2", "SDL2main"}
    

    include("lib.lua")
    --include("engine.lua")
    --include("scriptcore.lua")