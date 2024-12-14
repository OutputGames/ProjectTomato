from pyassimp import load
import numpy as np
import os
import pathlib
import shutil

def convertPosix(path):
    p = pathlib.PureWindowsPath(path)
    return p.as_posix()



def model_to_header(modelPath):

    headerPath = f"include/tomato_generated/{os.path.splitext(os.path.basename(modelPath))[0]}.h"
    outPath = os.path.dirname(headerPath)

    if not os.path.exists(outPath):
        os.makedirs(outPath)

    modelScale = 0.5

    with load(modelPath) as scene:
        with open(headerPath, "w") as f:

            id = (os.path.splitext(os.path.basename(headerPath))[0]).upper()

            f.write(f"#ifndef {id}_MESH_H\n")
            f.write(f"#define {id}_MESH_H\n\n")
            f.write(f"#include \"vertex.h\"\n\n")

            f.write("namespace {0}_mesh {{\n".format( os.path.splitext(os.path.basename(modelPath))[0]))

            j = 0
            for mesh in scene.meshes:
                f.write("\tstatic tmt::render::Vertex vertices_{0}[] = {{\n".format(j))
                for i in range(len(mesh.vertices)):
                    vertex = mesh.vertices[i]
                    normal = mesh.normals[i]
                    uv = mesh.texturecoords[0][i]
                    f.write("\t    { ")
                    f.write("glm::vec3{ ")
                    f.write(f"{vertex[0]*modelScale}, {vertex[1]*modelScale}, {vertex[2]*modelScale}")
                    f.write("}, ")

                    f.write("glm::vec3{ ")
                    f.write(f"{normal[0]}, {normal[1]}, {normal[2]}")
                    f.write("}, ")

                    f.write("glm::vec2{ ")
                    f.write(f"{uv[0]}, {uv[1]}")
                    f.write("} ")

                    f.write("},\n")
                    #f.write(f"\t    {{ glm::vec3{{ {vertex[0]}, {vertex[1]}, {vertex[2]} }} }},\n")
                f.write("\t};\n\n")

                f.write("\tstatic uint16_t indices_{0}[] = {{\n".format(j))
                for face in mesh.faces:
                    f.write(f"\t    {face[0]}, {face[1]}, {face[2]},\n")
                f.write("\t};\n\n")

                f.write("\tstatic uint16_t vertexCount_{0} = {1};\n".format(j, len(mesh.vertices)))
                f.write("\tstatic uint16_t indexCount_{0} = {1};\n\n".format(j, len(mesh.faces)*3))

                j += 1

            f.write("};\n\n")

            f.write("#endif\n")

for root, dirs, files in os.walk("resources/internal/models/"):
    for file in files:
        file_path = os.path.join(root, file)
        file_path = convertPosix(file_path)
        if (file_path.endswith(".obj")):
            model_to_header(file_path)

