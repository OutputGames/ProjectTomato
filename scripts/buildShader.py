import os
import sys
import subprocess
import pathlib
import shutil

shaderc = "D:/Code/ImportantRepos/TomatoEngine/vendor/bgfx/.build/win64_vs2022/bin/shadercRelease.exe"

def convertPosix(path):
    p = pathlib.PureWindowsPath(path)
    return p.as_posix()

def buildShader(path,type, args, outPath):
    parName = os.path.basename(os.path.dirname(path))



    ext = ""

    if (type == "vertex"):
        ext = "cvbsh"
    elif (type == "fragment"):
        ext = "cfbsh"
    elif (type == "compute"):
        ext = "ccbsh"

    out = outPath + "/" + parName + "/" +  os.path.splitext(os.path.basename(path))[0] + f".{ext}"
    out = convertPosix(out)

    if not os.path.exists(os.path.dirname(out)):
        os.makedirs(os.path.dirname(out))

    varyPath = f"{os.path.dirname(path)}/varying.def.sc"
    p = pathlib.PureWindowsPath(varyPath)
    varyPath = p.as_posix()

    cmd = f"{shaderc} {args} --type {type} --depends -o {out} -f {path} --disasm --varyingdef {varyPath}"

    '''
    print(path)

    with open(varyPath, "r") as file:
        for line in file:
            print(line, end="")

    print("/t")

    '''

    #print(cmd)

    #print(varyPath)

    os.system(cmd)

def build(platform):
    print(f"Building for {platform}")
    outPath = f"runtime/shaders/{platform}"

    if not os.path.exists(outPath):
        os.makedirs(outPath)
    else:
        shutil.rmtree(outPath)
        os.makedirs(outPath)

    vsf = "-i vendor/bgfx/src/ "
    fsf = "-i vendor/bgfx/src/ "
    csf = "-i vendor/bgfx/src/ "

    if (platform == "dx"):
        vsf += "-platform windows -p s_5_0 -O 3"
        fsf += "-platform windows -p s_5_0 -O 3"
        csf += "-platform windows -p s_5_0 -O 3"
    elif (platform == "gl"):
        vsf += "-platform linux -p 440"
        fsf += "-platform linux -p 440"
        csf += "-platform linux -p 440"

    for root, dirs, files in os.walk(os.getcwd()):
        for file in files:
            file_path = os.path.join(root, file)
            file_path = convertPosix(file_path)
            if (file.endswith(".vbsh") ):
                buildShader(file_path, "vertex",vsf, outPath)
            if (file.endswith(".fbsh") ):
                buildShader(file_path, "fragment",fsf, outPath)
            if (file.endswith(".cbsh") ):
                buildShader(file_path, "compute",csf, outPath)
    


platform = ""

if (len(sys.argv) > 1):
    platform = sys.argv[1]

if (platform == ""): platform = "all"

if (platform == "all"):
    build("dx")
    build("gl")
else:
    build(platform)