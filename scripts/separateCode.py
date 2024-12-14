import re
import os
import clang.cindex
import subprocess
import json
import pathlib


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
    dependencies = []
    raw_deps = []
    for child in node.get_children():
        s = child.type.spelling

        s = get_raw_type(s)

        #print(f" Name: {child.spelling}, Type: {child.type.spelling}, Kind:{child.kind}, Node:{node.spelling} ")


        if child.kind in (clang.cindex.CursorKind.FIELD_DECL, clang.cindex.CursorKind.CXX_BASE_SPECIFIER, clang.cindex.CursorKind.FRIEND_DECL ):  # Member variables
            #print(f"Name: {child.displayname}, Type: {child.type.spelling}, Kind:{child.kind}")
            if (child.kind is clang.cindex.CursorKind.FRIEND_DECL):
                children = child.get_children()
                for fc in children:
                    preRawType = fc.spelling.replace("struct tmt::", "")
                    rawType = get_raw_type(preRawType)
                    dependencies.append(rawType)
                    raw_deps.append(preRawType+"*")
            else:
                dependencies.append(s)
                raw_deps.append(child.type.spelling)
        elif child.kind == clang.cindex.CursorKind.STRUCT_DECL:
            dps = extract_dependencies(child)
            dependencies += dps[0]
            raw_deps += dps[1]
        elif child.kind in (clang.cindex.CursorKind.FUNCTION_DECL, clang.cindex.CursorKind.CXX_METHOD):
            #print(f"Name: {child.displayname}, Type: {child.type.spelling}, Kind:{child.kind}")
            dps = extract_func_dependencies(child)
            dependencies += dps[0]
            raw_deps += dps[1]

        if child.type.get_template_argument_type(0) is not None:
            for i in range(child.type.get_num_template_arguments()):
                template_arg = child.type.get_template_argument_type(i)
                if template_arg is not None:
                    dependencies.append(get_raw_type(template_arg.spelling))
                    raw_deps.append(template_arg.spelling)


    return [dependencies, raw_deps]

def extract_func_dependencies(node):
    dependencies = []
    raw_deps = []

    dependencies.append(get_raw_type(node.result_type.spelling))
    raw_deps.append(node.result_type.spelling)

    # Check if the return type is a template
    if node.result_type.get_template_argument_type(0) is not None:
        for i in range(node.result_type.get_num_template_arguments()):
            template_arg = node.result_type.get_template_argument_type(i)
            if template_arg is not None:
                dependencies.append(get_raw_type(template_arg.spelling))
                raw_deps.append(template_arg.spelling)


    for arg in node.type.argument_types():
        dependencies.append(get_raw_type(arg.spelling))
        raw_deps.append(arg.spelling)


        # Check if the argument is a template
        if arg.get_template_argument_type(0) is not None:
            for i in range(arg.get_num_template_arguments()):
                template_arg = arg.get_template_argument_type(i)
                if template_arg is not None:
                    dependencies.append(get_raw_type(template_arg.spelling))
                    raw_deps.append(template_arg.spelling)

    return [dependencies, raw_deps]

def resolve_include_for_type(type_name, structs, current_header_path=None):
    """Map a type to its corresponding header file, avoiding self-inclusion."""
    for [pkey, strc] in structs.items():
        for key, [decl, _] in strc.items():
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

def describe_constructor(constructor_cursor):
    #if constructor_cursor.kind == clang.cindex.CursorKind.CONSTRUCTOR:
    params = []
    for param in constructor_cursor.get_arguments():
        param_type = param.type.spelling
        param_name = param.spelling or "<unnamed>"
        params.append(f"{param_type}")

    param_list = ", ".join(params)
    return f"{constructor_cursor.spelling}({param_list})"

def sort_dict_by_key_position(dict, order):

    return {key : dict[key] for key in order if key in dict}

def split_cpp_files(header_file, source_file, output_dir):
    os.makedirs(output_dir, exist_ok=True)

    with open(header_file, 'r') as h_file:
        header_content = h_file.readlines()

    with open(source_file, 'r') as s_file:
        source_content = s_file.readlines()

    stdHeaders = "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.42.34433/include"

    absVendor = os.path.abspath("../../vendor/")

    absVendor = absVendor.replace("d:\\", "D:\\")

    glmPath = f"{absVendor}\\glm\\glm\\"

    #print(glmPath)

    args = ["-std=c++20", "-fsyntax-only", "-nostdinc", "-nostdinc++", f"-I{glmPath}"]

    header_unit = index.parse(header_file, args=args)
    source_unit = index.parse(source_file, args=args)

    structs = {}
    funcs = {}
    globals = []
    includes = {}
    declarations = {}

    #print(header_content)

    hc = header_unit.cursor.get_children()

    keys = []

    funcTypes = (clang.cindex.CursorKind.CXX_METHOD, clang.cindex.CursorKind.FUNCTION_DECL, clang.cindex.CursorKind.CONSTRUCTOR, clang.cindex.CursorKind.DESTRUCTOR)
    structor = (clang.cindex.CursorKind.CONSTRUCTOR, clang.cindex.CursorKind.DESTRUCTOR)

    for node in hc:
        if (node.kind == clang.cindex.CursorKind.NAMESPACE):
            for namespace in node.get_children():
                for child in namespace.get_children():
                    key = f"{node.spelling}::{namespace.spelling}::{child.spelling}"
                    pkey = f"{node.spelling}::{namespace.spelling}"

                    if (key in keys):
                        keys.remove(key)

                    keys.append(key)

                    if (child.spelling == "Vertex"):
                        continue

                    if not child.spelling in includes:
                        includes[child.spelling] = []
                        declarations[child.spelling] = []

                    if (child.kind in (clang.cindex.CursorKind.STRUCT_DECL, clang.cindex.CursorKind.ENUM_DECL)):

                        if not pkey in structs:
                            structs[pkey] = {}

                        structs[pkey][key] = [child, []]

                        #print(key)
                        for prop in child.get_children():
                            #print(f"\tName: {prop.spelling}, Kind: {prop.kind}")
                            if (prop.kind in funcTypes):

                                prop.realSpelling = describe_constructor(prop)
                                #rint(f"Appened property method: {prop.realSpelling}")

                                structs[pkey][key][1].append(prop)

                    else:
                        if not pkey in funcs:
                            funcs[pkey] = [namespace,[ [], [] ]]
                        funcs[pkey][1][0].append(child)
    
    for node in source_unit.cursor.get_children():

        if (node.spelling in ("randomInt", "randomFloat")):
            continue

        clsN = node.semantic_parent.spelling

        #print(f"Node kind: {node.kind}, Name: {node.spelling}")
        if (node.kind in funcTypes):

            key = get_full_path(node)
            pkey = get_full_path(node)
            if (node.kind == clang.cindex.CursorKind.CXX_METHOD):
                pkey = pkey.replace(f"::{clsN}", "")
            else:
                pkey = pkey.replace(f"::{node.spelling}", "")

            #print(f"pkey: {pkey}, key: {key}, {clsN}")

            if (node.kind == clang.cindex.CursorKind.CONSTRUCTOR):
                key += "::" + node.spelling
            elif node.kind == clang.cindex.CursorKind.DESTRUCTOR:
                key += "::~" + node.spelling

            node.realSpelling = describe_constructor(node)

            if (pkey in structs and key in structs[pkey]):
                rem = False
                midx = 0
                for prop in structs[pkey][key][1]:
                    #print(prop)
                    if (node.realSpelling == prop.realSpelling):
                        #print(f"\tFound unnatural in class {node.spelling}")
                        #print(f"\tReal type: {prop.kind}")
                        if (prop.kind != clang.cindex.CursorKind.VAR_DECL):
                            rem = True
                        else:
                            print("Predefined as variable, method will be unnatural")
                        break

                    midx += 1
                if (rem):
                    #print(f"Copied source defined node: {node.realSpelling}")
                    structs[pkey][key][1][midx] = node
                    continue
                else:
                    print(f"Found unnatural method: {node.realSpelling} ({key})")

            if (pkey in funcs):

                funcs[pkey][1][1].append(node)
            else:
                if (node.kind == clang.cindex.CursorKind.FUNCTION_DECL):
                    #print(f"Node kind: {node.kind}, Class Name: {clsN}, Name: {node.spelling}, Path: {key}, PPath: {pkey}")
                    globals.append(node)
                else:
                    continue
        elif (node.kind == clang.cindex.CursorKind.VAR_DECL):

            key = get_full_path(node) + "::" + node.spelling
            pkey = get_full_path(node)
            upkey = pkey.replace(f"::{clsN}", "")

            node.realSpelling = describe_constructor(node)

            if (clsN != "tomato.cpp" ):
                rem = False
                midx = 0
                if upkey in structs:
                    #print(f"\tFound namespace in struct {upkey}")
                    if pkey in structs[upkey]:
                        #print(f"\tFound class in namespace {pkey}")
                        for prop in structs[upkey][pkey][1]:
                            #print(prop)
                            if (prop.spelling == node.spelling):
                                #print(f"\tFound unnatural in class {node.spelling}")
                                #print(f"\tReal type: {prop.kind}")
                                if (prop.kind != clang.cindex.CursorKind.VAR_DECL):
                                    rem = True
                                    break
                            midx += 1
                if (rem):
                    print(f"Found unnatural variable: {node.spelling} ({key})")
                    structs[upkey][pkey][1][midx] = node
                    continue

            #print(f"pkey: {pkey}, key: {key}, {clsN}")

            #print(f"Node kind: {node.kind}, Class Name: {clsN}, Name: {node.spelling}, Path: {key}, PPath: {pkey}")
            globals.append(node)
        
    for namespace in structs.keys():
        dct = structs[namespace]
        structs[namespace] = sort_dict_by_key_position(dct, keys)

    # globals

    with open("globals.hpp", "w") as f:
        globalCode = '#ifndef GLOBALS_HPP\n #define GLOBALS_HPP\n\n'

        globalCode += '#include "utils.hpp"\n #include "tomato.hpp"\n'

        last_def = False
        for l in source_content:

            if ("_tomato.hpp" in l):
                continue

            if (l.startswith("#include")):
                globalCode += l
                if (last_def):
                    globalCode += "#endif\n"
                last_def = False
            if (l.startswith("#define")):
                globalCode += f"#ifndef {l.split("define ")[1]}"
                last_def = True

        globalCode += "\n"

        for g in globals:
            #print(g.spelling)
            if (g.kind == clang.cindex.CursorKind.FUNCTION_DECL):
                cd = get_code(g,source_content).split(")")[0]
                globalCode += cd + ");\n"
            else:

                if (g.semantic_parent.spelling != "tomato.cpp"):
                    continue

                cd = get_code(g,source_content).split(" = ")[0]

                if (not cd.startswith("static ")):
                    cd = "extern "+cd

                globalCode += cd + ";\n"
                #print(f"\t{g.semantic_parent.spelling}")
            #print(f"\t{cd}")
        globalCode += "\n\n#endif"

        f.write(globalCode)

    with open("globals.cpp", "w") as f:
        globalCode = ''

        last_def = False
        for l in source_content:

            if ("_tomato.hpp" in l):
                continue

            if (l.startswith("#include") and last_def):
                globalCode += l
                last_def = False
            if (l.startswith("#define")):
                globalCode += l
                last_def = True

        globalCode += "\n#include globals.hpp\n"

        globalCode += "\n"

        for g in globals:
            #print(g.spelling)

            cd = get_code(g,source_content)


            if (g.kind == clang.cindex.CursorKind.FUNCTION_DECL):
                cd1 = get_code(g,source_content).split(")")[0]

                regex = r"= [^,)\n]+"
                cleaned_decl = re.sub(regex, "", cd1)
                
                cd = cd.replace(cd1, cleaned_decl)
            else:
                #if "*" in cd:
                    #continue
                if (cd.startswith("static ")):
                    cd = cd.replace("static ", "")

            globalCode += cd
            globalCode += ";\n"

        f.write(globalCode)

    # non-specified functions
    hp = {}
    sp = {}
    hdp = {}
    hrdp = {}
    hpaths = []
    for [key, [nmsp, [dec,dfs] ]] in funcs.items():
        #print(nmsp.spelling)

        outPath = f"{output_dir}/{nmsp.spelling.capitalize()}/"

        hPath = outPath + f"{nmsp.spelling}.hpp"
        sPath = outPath + f"{nmsp.spelling}.cpp"

        os.makedirs(outPath, exist_ok=True)

        headerCode = ""
        sourceCode = ""


        dependencies = []
        raw_deps = []

        for _dc in dec:
            #print(d.kind)
            headerCode += get_code(_dc, header_content)
            headerCode += ";\n\n"
            fdeps = extract_func_dependencies(_dc)
            dependencies += fdeps[0]
            raw_deps += fdeps[1]

        for _df in dfs:
            _co = get_code(_df, source_content)
            sourceCode += _co
            sourceCode += "\n\n"

        headerCode = format_code(headerCode)
        sourceCode = format_code(sourceCode)

        hp[key] = headerCode
        sp[key] = sourceCode
        hdp[key] = dependencies
        hrdp[key] = raw_deps

        hpaths.append(f'#include "{hPath}"')

    # specified structs
    for [nmsp, strcts] in structs.items():

        ns = nmsp.split("::")[1]

        outPath = f"{output_dir}/{ns.capitalize()}/"

        hPath = outPath + f"{ns}.hpp"
        sPath = outPath + f"{ns}.cpp"

        incp = f'#include "{hPath}"'

        if (incp not in hpaths):
            hpaths.append(incp)

        os.makedirs(outPath, exist_ok=True)

        headerCode = ""
        
        sourceCode = ""

        #print(nmsp)

        defins = ""

        dependencies = []
        raw_dependencies = []
        for [key,[decl, defs]] in strcts.items():
            #print(key, nmsp)

            cd = get_code(decl, header_content)

            headerCode += cd
            headerCode += ";\n\n"

            dps, rdps = extract_dependencies(decl)

            dependencies += dps
            raw_dependencies += rdps

            for d in defs:
                fullKey = f"{key}::{d.realSpelling}"
                #print(f"{key}::{d.realSpelling}")
                cod = ""
                if (d.kind == clang.cindex.CursorKind.VAR_DECL):
                    cod = ""
                    _cd = get_code(d, source_content)
                    opened = False
                    for l in source_content:
                        if (l.startswith(_cd)):
                            opened = True
                            #print("Found opening")
                        if (opened):
                            cod += l + "\n"
                            if "}" in l:
                                #print("Closed")
                                opened = False
                                break
                    #print(cod)
                        
                else:
                    cod = get_code(d, source_content)

                    if key not in cod:
                        #print(f"\tFalse code: {key}::{d.realSpelling}")

                        #cod = get_code(d, header_content)

                        #print(cod)

                        cod = ""   

                if (cod != "" and cod not in sourceCode):
                    sourceCode += cod
                    sourceCode += "\n\n"

            knd = "struct"

            if (decl.kind == clang.cindex.CursorKind.ENUM_DECL):
                knd = "enum"

            defins += (f"{knd} {decl.spelling};\n")

        if nmsp in hdp:
            dependencies += hdp[nmsp]
            raw_dependencies += hrdp[nmsp]

        #print(hPath)
        include_statements = []
        pre_defs = []
        for i in range(0,len(dependencies)):
            dep = dependencies[i]
            rdep = raw_dependencies[i]
            if "::" in dep:
                nms = dep.split("::")[0]
                cls = dep.split("::")[1]
                #print(nms)
                if f"tmt::{nms}" in structs:
                    if "*" in rdep:
                        pdef = f"namespace tmt::{nms} {{\n struct {cls};\n }}\n"
                        #print(pdef)
                        pre_defs.append([pdef, nms])
                    else:
                        include_statements.append(f'#include "tomato/{nms.capitalize()}/{nms}.hpp"')
        rmd = []
        for [d,nms] in pre_defs:
            incPath = f'#include "tomato/{nms.capitalize()}/{nms}.hpp"'

            if incPath not in include_statements:
                rmd.append(d)

        pre_defs = rmd

        headerCode = defins + "\n" + headerCode

        if nmsp in sp:
            sourceCode += sp[nmsp] + "\n\n"
            sp.pop(nmsp)
        if nmsp in hp:
            headerCode += hp[nmsp] + "\n\n"
            hp.pop(nmsp)

        headerCode = format_code(headerCode)
        sourceCode = format_code(sourceCode)

        with open(hPath, "w") as f:
            f.write(f"#ifndef {ns.upper()}_H\n")
            f.write(f"#define {ns.upper()}_H\n\n")

            f.write("#include \"utils.hpp\" \n")

            f.write("\n".join(include_statements) + "\n\n")

            f.write("\n")

            f.write("\n".join(pre_defs) + "\n\n")
            f.write(f"namespace tmt::{ns} {{\n\n")
            
            f.write(headerCode)

            f.write(";\n\n}")

            f.write("\n\n#endif")
                
        with open(sPath, "w") as f:
            f.write(f"#include \"{ns}.hpp\" \n")

            f.write("#include \"globals.hpp\" \n")

            f.write("\n")

            f.write(sourceCode)

    for nmsp in hp.keys():

        ns = nmsp.split("::")[1]

        outPath = f"{output_dir}/{ns.capitalize()}/"

        hPath = outPath + f"{ns}.hpp"
        sPath = outPath + f"{ns}.cpp"

        incp = f'#include "{hPath}"'

        if (incp not in hpaths):
            hpaths.append(incp)

        os.makedirs(outPath, exist_ok=True)

        headerCode = ""
        
        sourceCode = ""

        #print(nmsp)

        defins = ""

        dependencies = []
        raw_dependencies = []

        #print(hPath)
        include_statements = []
        pre_defs = []
        for i in range(0,len(dependencies)):
            dep = dependencies[i]
            rdep = raw_dependencies[i]
            if "::" in dep:
                nms = dep.split("::")[0]
                cls = dep.split("::")[1]
                #print(nms)
                if f"tmt::{nms}" in structs:
                    if "*" in rdep:
                        pdef = f"namespace tmt::{nms} {{\n struct {cls};\n }}\n"
                        #print(pdef)
                        pre_defs.append([pdef, nms])
                    else:
                        include_statements.append(f'#include "tomato/{nms.capitalize()}/{nms}.hpp"')
        rmd = []
        for [d,nms] in pre_defs:
            incPath = f'#include "tomato/{nms.capitalize()}/{nms}.hpp"'

            if incPath not in include_statements:
                rmd.append(d)

        pre_defs = rmd

        headerCode = defins + "\n" + headerCode

        if nmsp in sp:
            sourceCode += sp[nmsp] + "\n\n"
        if nmsp in hp:
            headerCode += hp[nmsp] + "\n\n"

        headerCode = format_code(headerCode)
        sourceCode = format_code(sourceCode)
        
        with open(hPath, "w") as f:
            f.write(f"#ifndef {ns.upper()}_H\n")
            f.write(f"#define {ns.upper()}_H\n\n")

            f.write("#include \"utils.hpp\" \n")

            f.write("\n".join(include_statements) + "\n\n")

            f.write("\n")

            f.write("\n".join(pre_defs) + "\n\n")
            f.write(f"namespace tmt::{ns} {{\n\n")
            
            f.write(headerCode)

            f.write(";\n\n}")

            f.write("\n\n#endif")
                
        with open(sPath, "w") as f:
            f.write(f"#include \"{ns}.hpp\" \n")

            f.write("#include \"globals.hpp\" \n")

            f.write("\n")

            f.write(sourceCode)

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

    #print(os.path.abspath("../../build.bat"))


    bp = os.path.abspath("../../build.bat")
    subprocess.run([bp], cwd=os.path.dirname(bp))


# Example usage
split_cpp_files('_tomato.hpp', 'tomato.cpp', './')
