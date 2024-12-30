@echo off

cd ../

set shaderc=C:\Users\chris\TomatoEngine\vendor\bgfx\.build\win64_vs2022\bin\shadercRelease.exe
set plat=%1

IF "%plat%"=="" set plat=all

if %plat%==all (
    CALL :build dx
    CALL :build gl
    exit /b
) else ( CALL :build %plat% )

:build
set platform=%~1
set outPath=runtime/shaders/%platform%

mkdir "%outPath%"

echo Building for %platform%

if %platform%==dx (
    set vsf=%vsf% -platform windows -p s_5_0 -O 3
    set fsf=%fsf% -platform windows -p s_5_0 -O 3
    set csf=%csf% -platform windows -p s_5_0 -O 1
)

if %platform%==gl (
    set vsf=%vsf% -platform linux -p 120
    set fsf=%fsf% -platform linux -p 120
    set csf=%csf% -platform linux -p 430
)

set vsf=%vsf% -i vendor/bgfx/src/ --varyingdef vendor/bgfx/src/varying.def.sc
set fsf=%fsf% -i vendor/bgfx/src/ --varyingdef vendor/bgfx/src/varying.def.sc
set csf=%csf% -i vendor/bgfx/src/ --varyingdef vendor/bgfx/src/varying.def.sc

for /R %%f in (*.vbsh) do %shaderc% %vsf% --type vertex --depends -o %outPath%/%%~nf.cvbsh -f %%f --disasm --varyingdef %%~dpf/varying.def.sc
for /R %%f in (*.fbsh) do %shaderc% %fsf% --type fragment --depends -o %outPath%/%%~nf.cfbsh -f %%f --disasm --varyingdef %%~dpf/varying.def.sc
for /R %%f in (*.cbsh) do %shaderc% %csf% --type compute --depends -o %outPath%/%%~nf.ccbsh -f %%f --disasm --varyingdef %%~dpf/varying.def.sc

echo Built all shaders!
EXIT /B 0