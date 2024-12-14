import re
import os
import clang.cindex
import subprocess
import json



with open("scripts/CodeSeparateConfig.json", 'r') as h_file:
    config = json.loads(h_file.read())
    #print(config)

with open("scripts/CodeGeneratedIncludes.json", 'r') as h_file:
    genConfig = json.loads(h_file.read())
    #print(config)

os.chdir("include/tomato")

clang.cindex.Config.set_library_path("C:/ClangLLVM/bin")

index = clang.cindex.Index.create()

pathNodeTypes = (clang.cindex.CursorKind.NAMESPACE, clang.cindex.CursorKind.STRUCT_DECL, clang.cindex.CursorKind.CLASS_DECL, clang.cindex.CursorKind.CLASS_TEMPLATE, clang.cindex.CursorKind.CXX_METHOD, clang.cindex.CursorKind.ENUM_DECL, clang.cindex.CursorKind.FUNCTION_DECL, clang.cindex.CursorKind.VAR_DECL)

def get_full_path(node):
    path = []
    current = node
    while current is not None:
        if current.kind in pathNodeTypes:
            path.append(current.spelling)
        current = current.semantic_parent

    del path[0]
    return "::".join(reversed(path))

def get_paths(node):
    path = []
    current = node
    while current is not None:
        if current.kind in pathNodeTypes:
            path.append(current.spelling)
        current = current.semantic_parent
    del path[0]
    path.reverse()
    return path

def get_raw_type(s):
    c = [" *", " **", "*"]

    for ch in c:
        if (s.endswith(ch)):
            s = s.replace(ch, "")

    return s

def get_code(node, content):
    
    start = node.extent.start
    end = node.extent.end

    extr_lines = content[start.line - 1 : end.line]
    if (len(extr_lines) == 1):
        return extr_lines[0][start.column - 1 : end.column - 1]
    else:
        extr_lines[0] = extr_lines[0][start.column - 1 :]
        extr_lines[-1] = extr_lines[-1][: end.column - 1]
        return "".join(extr_lines)
    
def format_code(code, style="Microsoft"):
    try:
        result =     subprocess.run(["C:/ClangLLVM/bin/clang-format.exe", f"-style={style}"], input=code, text=True, capture_output=True, check=True)
        return result.stdout
    except subprocess.CalledProcessError as e:
        print(f"Error formatting code: {e}")
        return code
    
def extract_dependencies(node):
    dependencies = set()
    for child in node.get_children():
        s = child.type.spelling

        s = get_raw_type(s)

        #print(f" Name: {child.spelling}, Type: {child.type.spelling}, Kind:{child.kind}, Node:{node.spelling} ")


        if child.kind in (clang.cindex.CursorKind.FIELD_DECL, clang.cindex.CursorKind.CXX_BASE_SPECIFIER, clang.cindex.CursorKind.FRIEND_DECL ):  # Member variables
            dependencies.add(s)
            #print(f"Name: {child.spelling}, Type: {child.type.spelling}, Kind:{child.kind}")

        if child.type.get_template_argument_type(0) is not None:
            for i in range(child.type.get_num_template_arguments()):
                template_arg = child.type.get_template_argument_type(i)
                if template_arg is not None:
                    dependencies.add(get_raw_type(template_arg.spelling))
    return dependencies

def extract_func_dependencies(node):
    dependencies = set()

    dependencies.add(get_raw_type(node.result_type.spelling))

    # Check if the return type is a template
    if node.result_type.get_template_argument_type(0) is not None:
        for i in range(node.result_type.get_num_template_arguments()):
            template_arg = node.result_type.get_template_argument_type(i)
            if template_arg is not None:
                dependencies.add(get_raw_type(template_arg.spelling))


    for arg in node.type.argument_types():
        dependencies.add(get_raw_type(arg.spelling))


        # Check if the argument is a template
        if arg.get_template_argument_type(0) is not None:
            for i in range(arg.get_num_template_arguments()):
                template_arg = arg.get_template_argument_type(i)
                if template_arg is not None:
                    dependencies.add(get_raw_type(template_arg.spelling))

    return dependencies

def resolve_include_for_type(type_name, structs, current_header_path=None):
    """Map a type to its corresponding header file, avoiding self-inclusion."""
    for key, [decl, _] in structs.items():
        if decl.spelling == type_name or type_name.endswith(f"::{decl.spelling}"):
            paths = get_paths(decl)
            del paths[0]
            out_path = "tomato/"
            for path in paths:
                out_path += f"{path.capitalize()}/"
            resolved_path = out_path + decl.spelling + ".hpp"

            # Skip self-inclusion
            if current_header_path and os.path.abspath(resolved_path.replace("tomato/", "")) == os.path.abspath(current_header_path):
                print(f"Prevented self inclusion for {current_header_path}")
                continue

            return resolved_path
    return None

def get_struct_from_name(structs, name):

    for [s, v] in structs.items():
        if (s.split("::")[2] is name):
            return v
    return None

def split_cpp_files(header_file, source_file, output_dir):
    os.makedirs(output_dir, exist_ok=True)

    with open(header_file, 'r') as h_file:
        header_content = h_file.readlines()

    with open(source_file, 'r') as s_file:
        source_content = s_file.readlines()

    stdHeaders = "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.42.34433/include"

    header_unit = index.parse(header_file, args=["-std=c++20", f"-I{stdHeaders}"])
    source_unit = index.parse(source_file, args=["-std=c++20", f"-I{stdHeaders}"])

    structs = {}
    funcs = {}
    globals = []
    includes = {}
    declarations = {}

    #print(header_content)

    for node in header_unit.cursor.get_children():
        if (node.kind == clang.cindex.CursorKind.NAMESPACE):
            for namespace in node.get_children():
                for child in namespace.get_children():
                    key = f"{node.spelling}::{namespace.spelling}::{child.spelling}"
                    pkey = f"{node.spelling}::{namespace.spelling}"
                    #print(f"Key: {key}, Kind: {child.kind}")

                    if not child.spelling in includes:
                        includes[child.spelling] = []
                        declarations[child.spelling] = []

                    if (child.kind in (clang.cindex.CursorKind.STRUCT_DECL, clang.cindex.CursorKind.ENUM_DECL)):
                        structs[key] = [child, []]
                    else:
                        if not pkey in funcs:
                            funcs[pkey] = [namespace,[ [], [] ]]
                        funcs[pkey][1][0].append(child)
    
    for node in source_unit.cursor.get_children():
        #print(f"Node kind: {node.kind}, Name: {node.spelling}")
        if (node.kind in (clang.cindex.CursorKind.CXX_METHOD, clang.cindex.CursorKind.FUNCTION_DECL, clang.cindex.CursorKind.CONSTRUCTOR)):
            key = get_full_path(node)
            pkey = get_full_path(node).replace(f"::{node.spelling}", "")

            #print(f"pkey: {pkey}, {key}, {node.spelling}")

            if (node.kind == clang.cindex.CursorKind.CONSTRUCTOR):
                key += "::" + node.spelling

            #print(f"Node kind: {node.kind}, Name: {node.spelling}, Path: {key}")

            if (key in structs):
                structs[key][1].append(node)

            elif (pkey in funcs):
                funcs[pkey][1][1].append(node)
            else:
                continue
        elif (node.kind == clang.cindex.CursorKind.VAR_DECL):
            globals.append(node)
            if node.type.get_template_argument_type(0) is not None:
                print(f"{node.type.spelling} : {node.spelling}")
                for i in range(node.type.get_num_template_arguments()):
                    template_arg = node.type.get_template_argument_type(i)
                    if template_arg is not None:
                        print(template_arg.spelling)
                        globals.add(get_raw_type(template_arg.spelling))
        
    with open("globals.cpp", "w") as f:
        globalCode = '#include "utils.hpp" \n'

        globalIncludes = []

        for g in globals:
            s = get_raw_type(g.type.spelling)
            s = s.replace("tmt::", "")

            paths = s.split("::")

            if (len(paths) > 1):
                paths[0] = paths[0].capitalize()
                globalIncludes.append(paths)

            #print(get_raw_type(s))

        for i in globalIncludes:
            globalCode += f'#include "{"/".join(i)}.hpp" \n'

        globalCode += "\n"

        for g in globals:
            globalCode += get_code(g,source_content)
            globalCode += ";\n"

        f.write(globalCode)

    hpaths = []
    for [key, [nmsp, [decl,defs] ]] in funcs.items():
        #print(nmsp.spelling)

        outPath = f"{output_dir}/{nmsp.spelling.capitalize()}/"

        hPath = outPath + f"{nmsp.spelling}.hpp"
        sPath = outPath + f"{nmsp.spelling}.cpp"

        os.makedirs(outPath, exist_ok=True)

        headerCode = ""
        sourceCode = ""


        dependencies = set()

        for d in decl:
            #print(d.kind)
            headerCode += get_code(d, header_content)
            headerCode += ";\n\n"
            dependencies.update(extract_func_dependencies(d))

        for d in defs:
            sourceCode += get_code(d, source_content)
            sourceCode += "\n\n"

        headerCode = format_code(headerCode)
        sourceCode = format_code(sourceCode)

        hpaths.append(f'#include "{hPath}"')

        #print(sourceCode)
        include_statements = set()
        for dep in dependencies:
            include_path = resolve_include_for_type(dep, structs, hPath)
            if include_path:
                include_statements.add(f'#include "{include_path}"')

        if nmsp.spelling in config["includeOverrides"]:
            for p in config["includeOverrides"][nmsp.spelling]:
                include_statements.add(f'#include "{p}"')
        #include_statements.remove(ri)

        with open(hPath, "w") as f:
            f.write(f"#ifndef {nmsp.spelling.upper()}_H\n")
            f.write(f"#define {nmsp.spelling.upper()}_H\n\n")

            f.write("#include \"utils.hpp\" \n")

            f.write("\n".join(include_statements) + "\n\n")

            f.write("\n")

            f.write(f"namespace tmt::{nmsp.spelling} {{\n\n")
            
            f.write(headerCode)

            f.write(";\n\n}")

            f.write("\n\n#endif")
    
        with open(sPath, "w") as f:
            f.write(f"#include \"{nmsp.spelling}.hpp\" \n")

            f.write("#include \"globals.cpp\" \n")

            f.write("\n")

            f.write(sourceCode)

    for [key, [decl, defs]] in structs.items():

        #print(key)

        headerCode = get_code(decl, header_content)

        paths = get_paths(decl)

        del paths[0]

        c = []
        fdependencies = set()
        for d in defs:
            code = get_code(d, source_content)

            code = format_code(code)

            #print(code)
            c.append(code)

            fdependencies.update(extract_func_dependencies(d))

        outPath = f"{output_dir}/"

        for path in paths:
            #print(path)
            outPath += f"{path.capitalize()}/"

        hPath = outPath + f"{decl.spelling}.hpp"
        sPath = outPath + f"{decl.spelling}.cpp"

        hpaths.append(f'#include "{hPath}"')

        os.makedirs(outPath, exist_ok=True)

        headerCode = format_code(headerCode)

        dependencies = extract_dependencies(decl)
        dependencies.update(fdependencies)

        # Resolve includes for dependencies
        include_statements = []
        for dep in dependencies:
            include_path = resolve_include_for_type(dep, structs, hPath)
            if include_path:
                include_statements.append(f'#include "{include_path}"')

        if decl.spelling in config["includeOverrides"]:
            for p in config["includeOverrides"][decl.spelling]:
                include_statements.append(f'#include "{p}"')

        for i in include_statements:
            sourceInclude = i.replace('#include ', "")
            sourceInclude = sourceInclude.replace('"', "")

            includes[decl.spelling].append(sourceInclude)

        dclrs = []
        for d in genConfig["declares"][decl.spelling]:
            strc = get_struct_from_name(structs, d)
            include_path = resolve_include_for_type(d, structs, hPath)
            if include_path:
                include_statements.remove(f'#include "{include_path}"')

                include_path = include_path.replace("tomato/", "tmt::")

                nms = include_path.split("/")[0]
                nms = nms.lower()

                dclrs.append(f"namespace {nms} {{\n struct {d};\n }};\n")

        with open(hPath, "w") as f:
            f.write(f"#ifndef {decl.spelling.upper()}_H\n")
            f.write(f"#define {decl.spelling.upper()}_H\n\n")

            f.write("#include \"utils.hpp\" \n")

            f.write("\n".join(dclrs))

            f.write("\n".join(include_statements) + "\n\n")

            f.write("\n")

            f.write(f"namespace tmt::{"::".join(paths)} {{\n\n")
            
            f.write(headerCode)

            f.write(";\n\n}")

            f.write("\n\n#endif")
        
        with open(sPath, "w") as f:
            f.write(f"#include \"{decl.spelling}.hpp\" \n")

            f.write("#include \"globals.cpp\" \n")

            f.write("\n")

            for cf in c:

                #print(cf)

                f.write(cf)
                f.write("\n\n")

    with open("tomato.hpp", "w") as f:
        f.write(f"#ifndef TM_H\n")
        f.write(f"#define TM_H\n\n")

        f.write("#include \"utils.hpp\" \n")

        f.write("\n".join(hpaths) + "\n\n")

        f.write("#endif")

    with open("D:/Code/ImportantRepos/TomatoEngine/scripts/CodeGeneratedIncludes.json", "w") as f:

        for [cls, incs] in includes.items():
            for i in range(0, len(incs)):
                raw_inc = re.search("tomato/.*/(.*).hpp", incs[i]).group(1)
                incs[i] = raw_inc

        for [cls, incs] in includes.items():
            for i in incs:
                o_cls = includes[i]
                if (cls in o_cls):
                    print(f"Recursive self inclusion found in: {cls}:{i}")
                    includes[i].remove(cls)
                    declarations[i].append(cls)

        j = { 
            "includes": includes,
            "declares": declarations 
        }

        f.write(json.dumps(j, sort_keys=True, indent=2))

    print(f"Classes and structs split into separate files in '{output_dir}'.")



# Example usage
split_cpp_files('_tomato.hpp', 'tomato.cpp', './')
