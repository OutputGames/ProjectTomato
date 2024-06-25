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

    include("lib.lua")
    include("engine.lua")
    include("scriptcore.lua")