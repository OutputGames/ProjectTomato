#include "script.h"

#include "engine.hpp"
#include "util/filesystem_tm.h"
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/threads.h>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/object.h>
#include <mono/metadata/class.h>

CLASS_DEFINITION(Component, MonoComponent)

float get_time() { return tmEngine::tmTime::time; }

void custom_log_handler(const char* log_domain, const char* log_level, const char* message, mono_bool fatal, void* user_data) {
    std::cout << "[" << log_level << "] " << message << std::endl;
}

ScriptMgr::ScriptMgr()
{
    //#pragma region Init mono runtime
    mono_set_dirs(MONO_HOME "/lib",
        MONO_HOME "/etc");

    Logger::logger << "Set mono directories." << std::endl;

    MonoDomain* rootDomain = mono_jit_init("TomatoRuntime");
    if (rootDomain == nullptr)
    {
        // Maybe log some error here
        return;
    }
    Logger::logger << "Initialized mono runtime." << std::endl;

    // Store the root domain pointer
    s_RootDomain = rootDomain;

    // Create an App Domain
    s_AppDomain = mono_domain_create_appdomain(R"(TomatoAppDomain)", nullptr);
    mono_domain_set(s_AppDomain, true);
    Logger::logger << "Initialized mono domain" << std::endl;

    mono_add_internal_call("TomatoEngine.InternalCalls::GetTransformValue", &GetTransformValue);
    mono_add_internal_call("TomatoEngine.InternalCalls::SetTransformValue", &SetTransformValue);
    mono_add_internal_call("Tomato.Time::get_time", &get_time);
    Logger::logger << "Assigned internal calls." << std::endl;

    mono_trace_set_level_string("debug");
    mono_trace_set_log_handler(custom_log_handler, NULL);
    Logger::logger << "Initialized debugger." << std::endl;

}

ScriptMgr::~ScriptMgr()
{
    /*
    // Invoke garbage collection to clean up managed objects
    mono_gc_collect(mono_gc_max_generation());

    // Wait for any pending finalizers to complete
    while (mono_gc_pending_finalizers())
    {
	    
    }
    */

    mono_domain_unload(s_AppDomain);
    // Uninitialize the Mono runtime
    mono_jit_cleanup(s_RootDomain);
}

ScriptMgr::tsAssembly* ScriptMgr::LoadAssembly(string assemblyPath)
{

    /*
    if (assemblies.Contains(assemblyPath))
    {
        delete assemblies[assemblyPath];
        assemblies.Remove(assemblyPath);
        wasReloaded = true;
    }
    */

    //std::cout << "Loading assembly at " << assemblyPath << std::endl;

    if (!std::filesystem::exists(assemblyPath)) {
        Logger::logger << "Assembly does not exist at " << assemblyPath << "!\n";
        return nullptr;
    }

    uint32_t fileSize = 0;
    char* fileData = tmfs::loadFileBytes(assemblyPath, fileSize);

    // NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
    MonoImageOpenStatus status;
    MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

    if (status != MONO_IMAGE_OK)
    {
        const char* errorMessage = mono_image_strerror(status);
        // Log some error message using the errorMessage data
        return nullptr;
    }

    MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
    mono_image_close(image);

    var tmAssemb = new tsAssembly(assembly, this);
    tmAssemb->path = assemblyPath;

    assemblies[assemblyPath] = tmAssemb;

    // Don't forget to free the file data
    delete[] fileData;

    return tmAssemb;
}

void ScriptMgr::CompileFull()
{
    string monoPath = MONO_HOME "bin/";
    string msBuildPath = MSBUILD_HOME;


    try {
        // Remove the directory and all its contents
        std::error_code ec;
        auto count = fs::remove_all("./temp/build/", ec);

        if (ec) {
            std::cerr << "Error removing directory: " << ec.message() << '\n';
        }
        else {
            Logger::logger << "Successfully removed " << std::to_string(count) << " items.\n";
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << '\n';
    }
    catch (const std::exception& e) {
        std::cerr << "General exception: " << e.what() << '\n';
    }

    std::filesystem::create_directories("./temp/build/");

    string projectName = "TomatoApp";

    string mainTemplate = tmfs::loadFileString("projectsys/proj_main.cs");

    string premakeTemplate = tmfs::loadFileString("projectsys/generate_proj.lua");

    premakeTemplate = ReplaceAll(premakeTemplate, "_PROJECTNAME", projectName);
    mainTemplate = ReplaceAll(mainTemplate, "_PROJECTNAME", projectName);

    //std::remove_if(mainTemplate.begin(), mainTemplate.end(), isspace);

    tmfs::writeFileString("temp/build/Main.cs", mainTemplate);
    tmfs::writeFileString("temp/build/premake5.lua", premakeTemplate);
    tmfs::copyFile("projectsys/premake5.exe", "temp/build/premake5.exe");


    std::filesystem::path exePath = std::filesystem::current_path();

#ifdef DEBUG
    exePath = std::filesystem::path(exePath.string() + "\\bin\\Debug\\");
#endif


    // Vector to store the paths of DLL files
    std::vector<std::filesystem::path> dllFiles;

    // Iterate through the directory contents
    for (const auto& entry : std::filesystem::directory_iterator(exePath)) {
        // Check if the entry is a regular file and has a .dll extension
        if (entry.is_regular_file() && entry.path().extension() == ".dll") {
            dllFiles.push_back(entry.path());
        }
    }

    RunCommand("cd temp/build/ && premake5 vs2022");

    RunCommand(("cd "+msBuildPath+" && MsBuild "+std::filesystem::absolute("temp/build/"+projectName+".sln").string()).c_str());

    tmfs::copyFile("bin/Debug/TomatoRuntime.dll", "temp/build/Bin/Debug/TomatoRuntime.dll");

    for (auto dll_file : dllFiles) {
        Logger::logger << dll_file.string() << std::endl;
        tmfs::copyFile(dll_file.string(), "temp/build/Bin/Debug/" + dll_file.filename().string());
    }

    tmfs::writeFileString("temp/build/Bin/Debug/game.tmg", tmeGetCore()->SerializeGame());

    tmfs::copyDirectory("resources/", "temp/build/Bin/Debug/resources/");

    fs::create_directory("temp/build/Bin/Debug/scriptcore");
    tmfs::copyFile("scriptcore/TomatoScript.dll", "temp/build/Bin/Debug/scriptcore/TomatoScript.dll");

    RunCommand(("cd temp/build/Bin/Debug/ && start " + projectName + ".exe").c_str());

}

void ScriptMgr::Update()
{;
    if (mainAssembly)
    {
	    
    }
    wasReloaded = false;
}

void ScriptMgr::Reload()
{
    List<string> assemblyPaths;

    for (auto assembly : assemblies)
        assemblyPaths.Add(assembly.first);
    string mainAssemblyPath = mainAssembly->path;

    Unload();

    //const cast because method expects a char* for some reason
    s_AppDomain = mono_domain_create_appdomain(R"(TomatoAppDomain)", nullptr);
    mono_domain_set(s_AppDomain, true);

    for (auto assembly_path : assemblyPaths) {
        var assem = LoadAssembly(assembly_path);
        if (assembly_path == mainAssemblyPath)
            mainAssembly = assem;
    }


    wasReloaded = true;
    
}

void ScriptMgr::Unload()
{
    mono_domain_set(mono_get_root_domain(), true);
    mono_domain_unload(s_AppDomain);
    assemblies.Clear();
}


MonoComponent::MonoComponent(int componentIndex, int assemblyIndex) : Component()
{
    Initialize(componentIndex, assemblyIndex);
}

MonoComponent::MonoComponent(const char* componentName, int assemblyIndex): MonoComponent(
	tmeGetCore()->scriptMgr->assemblies.GetVector()[assemblyIndex].second->GetMonoComponentIndex(componentName), assemblyIndex)
{

}

void MonoComponent::RuntimeStart()
{
	Component::RuntimeStart();

    var eng = tmeGetCore();

	auto ts_assembly = eng->scriptMgr->mainAssembly;
	var actorClass = ts_assembly->GetClass("TomatoScript.Tomato", "Actor");
    var actorObj = actorClass->CreateInstance();
    var actor = GetActor();

    actorObj->SetPropertyValue("Name", actor->name);
    actorObj->SetFieldValue("_actorId", &actor->id);

    obj_->SetFieldValue("_actor", actorObj->GetObject());

    var transformObj = ts_assembly->GetClass("TomatoScript.Tomato", "Transform")->CreateInstance();

    var vec3Obj = ts_assembly->GetClass("Tomato", "Vector3");

    transformObj->SetPropertyValue("Actor", actorObj->GetObject());;
    actorObj->SetPropertyValue("Transform", transformObj->GetObject());
    obj_->SetFieldValue("_transform", transformObj->GetObject());

    obj_->CallMethod("Start");
}


#define checkTypes(t, c) mono_metadata_type_equal(t, mono_class_get_type(c))

void MonoComponent::RuntimeUpdate()
{

    if (tmeGetCore()->scriptMgr->wasReloaded)
    {
        class_ = nullptr;
        obj_ = nullptr;
        assembly_ = tmeGetCore()->scriptMgr->assemblies[assemblyPath];
        Initialize(className.c_str(), tmeGetCore()->scriptMgr->assemblies.IndexOf(assemblyPath));
		RuntimeStart();
    }

	Component::RuntimeUpdate();

    //Logger::Log("updated");

    for (auto field : fields.GetVector())
    {

        void* data = nullptr;

        if (checkTypes(field->type, mono_get_single_class()) || checkTypes(field->type, mono_get_double_class()))
        {
            data = &field->data.float_0;
        }
        if (checkTypes(field->type, mono_get_int16_class()) || checkTypes(field->type, mono_get_int32_class()) || checkTypes(field->type, mono_get_int64_class()))
        {
            data = &field->data.int_0;
        }
        if (checkTypes(field->type, mono_get_boolean_class()))
        {
            data = &field->data.bool_0;
        }

        obj_->SetFieldValue(field->name.c_str(), data);

    }

    obj_->CallMethod("Update");
}

#define monofield(type) f[#type "_0"] = field->data.type##_0;
#define monofieldset(type) field->data.type##_0 = mfield[#type "_0"];

void MonoComponent::Deserialize(nlohmann::json j)
{
    Initialize(j["comp_name"].get<string>().c_str());

    for (int i = 0; i < fields.GetCount(); ++i)
    {
        var field = fields[i];
        var mfield = j["fields"][field->name];

        monofieldset(int)
        monofieldset(float)
        monofieldset(bool)

        /*
        monofieldset(vec2)
        monofieldset(vec3)
        monofieldset(vec4)

        monofieldset(mat2)
        monofieldset(mat3)
        monofieldset(mat4)
        */

    }

    Component::Deserialize(j);
}


nlohmann::json MonoComponent::Serialize()
{
	nlohmann::json c = Component::Serialize();

    c["comp_name"] = className;

    for (auto field : fields.GetVector())
    {
        json f = {};
        f["name"] = field->name;

        monofield(int)
        monofield(float)
        monofield(bool)

        /*
        monofield(vec2)
        monofield(vec3)
        monofield(vec4)

        monofield(mat2)
        monofield(mat3)
        monofield(mat4)
        */


        c["fields"][field->name] = f;
    }

    return c;
}

void MonoComponent::EngineRender()
{

    for (auto field : fields.GetVector())
    {

        var typeName = mono_type_get_name(field->type);

        if (checkTypes(field->type, mono_get_single_class()) || checkTypes(field->type, mono_get_double_class()))
        {
            ImGui::DragFloat(field->name.c_str(), &field->data.float_0, 0.1);
        }
        if (checkTypes(field->type, mono_get_int16_class()) || checkTypes(field->type, mono_get_int32_class()) || checkTypes(field->type, mono_get_int64_class()))
        {
            ImGui::DragInt(field->name.c_str(), &field->data.int_0);
        }
        if (checkTypes(field->type, mono_get_boolean_class()))
        {
            ImGui::Checkbox(field->name.c_str(), &field->data.bool_0);
        }
    }

}

std::string MonoComponent::GetName()
{
    return className;
}

void MonoComponent::Initialize(int componentIndex, int assemIdx)
{
    if (componentIndex == -1) {
        Logger::logger << "Unable to get component with index: " + componentIndex << std::endl;
        return;
    }

    var eng = tmeGetCore();

    assemblyPath = eng->scriptMgr->assemblies.GetVector()[assemIdx].first;
    assembly_ = eng->scriptMgr->assemblies.GetVector()[assemIdx].second;
    class_ = eng->scriptMgr->assemblies.GetVector()[assemIdx].second->monoComponents[componentIndex];
    obj_ = class_->CreateInstance();

    className = class_->name;

    fields.Clear();
    for (auto field : class_->fields)
    {

        if (field->accessibility & (u8)ScriptMgr::Accessibility::Public) {

            var mfield = new MonoField();
            mfield->name = field->name;

            var type = mono_field_get_type(field->field_);

            mfield->type = type;

            mfield->data.int_0 = 0;
            mfield->data.float_0 = 0;
            mfield->data.bool_0 = false;

            mfield->data.vec2_0 = vec2(0);
            mfield->data.vec3_0 = vec3(0);
            mfield->data.vec4_0 = vec4(0);

            mfield->data.mat2_0 = mat2(0);
            mfield->data.mat3_0 = mat3(0);
            mfield->data.mat4_0 = mat4(0);

            fields.Add(mfield);
        }

    }

    Logger::logger << "Initialized scripted component: " << class_->name << std::endl;
}

ScriptMgr::tsAssembly::tsAssembly(MonoAssembly* assembly, ScriptMgr* script_mgr)
{
    this->assembly = assembly;
    this->scriptMgr = script_mgr;
    image = mono_assembly_get_image(assembly);

    const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
    int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

    for (int32_t i = 0; i < numTypes; i++)
    {
        uint32_t cols[MONO_TYPEDEF_SIZE];
        mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

        string nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
        string name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

        types.push_back(nameSpace + "." + name);

        var klass = GetMonoClass(nameSpace, name);

        //printf("%s.%s\n", nameSpace.c_str(), name.c_str());

        if (!klass)
        {
            printf("\tClass does not exist.\n\n");
            return;
        }

        var tmClass = new tsClass(klass);
        tmClass->assembly = this;

        classes.push_back(tmClass);


        //printf("\n");

    }

    Logger::logger << "Initialized " << std::to_string(classes.size()) << " mono classes." << std::endl;

    for (auto class_ : classes) {
        if (class_->parentClass) {
            class_->parentTsClass = GetClass(class_->parentClass);

        }

        if (class_->parentTsClass) {
            if (class_->parentTsClass->name == "MonoComponent")
            {
                components.push_back(class_);
                monoComponents.push_back(class_);
            }
            else if (class_->parentTsClass->name == "Component")
            {
                components.push_back(class_);
            }
        }

    }

    for (auto class_ : classes) {
        class_->GetProperties();
        class_->GetFields();
    }

    for (auto class_ : classes)
    {
        Logger::logger << class_->name << std::endl;

        string parentName = "Base Class";

        if (class_->parentTsClass)
            parentName = class_->parentTsClass->name;

        Logger::logger << "\tParent class: " << parentName << std::endl;

        for (auto field : class_->fields)
            Logger::logger << "\tField: " << field->name << std::endl;

        for (auto property : class_->properties)
            Logger::logger << "\tProperty: " << property->name << std::endl;

        void* itr = nullptr;

        Logger::logger << "\tMethods:" << std::endl;

        while (var method = mono_class_get_methods(class_->GetClass(), &itr))
        {
            class_->methods.push_back(method);

            Logger::logger << "\t\tMethod: " << mono_method_get_name(method) << std::endl;

            var sig = mono_method_get_signature(method, image, mono_method_get_token(method));

        	int paramCount = mono_signature_get_param_count(sig);

            if (paramCount > 0) {
                const char** paramNames = new const char* [paramCount];
                MonoType** params = new MonoType*[paramCount];

                mono_method_get_param_names(method, paramNames);

                void* itr2 = nullptr;

                int j = 0;
                while (var par = mono_signature_get_params(sig, &itr2))
                {
                    params[j] = par;
                    j++;
                }

                for (int i = 0; i < paramCount; ++i)
                {
                    var paramType = params[i];

                    Logger::logger << "\t\t\t" << mono_type_get_name(paramType) << " " << paramNames[i] << std::endl;
                }
            }
        }

        Logger::logger << std::endl;
    }


}

ScriptMgr::tsAssembly::~tsAssembly()
{
    //mono_image_close(image);
    mono_assembly_close(assembly);
    types.~vector();
    classes.~vector();
    monoComponents.~vector();
    components.~vector();
    //objects.~vector();

    
    for (auto object : objects)
        delete object;
}

ScriptMgr::tsAssembly::tsClass::tsClass(MonoClass* klass)
{
    this->_class = klass;

    nameSpace = mono_class_get_namespace(_class);
    name = mono_class_get_name(_class);

    void* itr = nullptr;

    itr = nullptr;

    itr = nullptr;

    /*
    while (var type = mono_class_get_nested_types(_class, &itr))
    {
        Logger::logger << "\tSubclass: " << mono_class_get_name(type) << std::endl;
    }
    */

    
    var type = mono_class_get_parent(_class);

    if (type) {
        parentClass = type;
        //Logger::logger << "\tParent class: " << mono_class_get_name(type) << std::endl;
    }


}

ScriptMgr::tsAssembly::tsClass::~tsClass()
{

}

ScriptMgr::tsAssembly::tsObject::tsObject(tsClass* klass, tsAssembly* assembly, void** params, int paramCt, bool autoCall)
{
    _assembly = assembly;
    object_ = mono_object_new(assembly->scriptMgr->s_AppDomain, klass->GetClass());
    assembly->objects.push_back(this);

    Class = klass;

    if (object_ == nullptr)
    {
        return;
    }

    if (autoCall) {
        //mono_runtime_object_init(object_);
        CallMethod(".ctor", params, paramCt);
    }
}

ScriptMgr::tsAssembly::tsObject::tsObject(MonoObject* object, tsAssembly* assembly, void** params, int paramCt, bool autoCall)
{
    object_ = object;
    _assembly = assembly;
    assembly->objects.push_back(this);

    Class = assembly->GetClass(mono_object_get_class(object));

    if (object_ == nullptr)
    {
        return;
    }

    if (autoCall)
    {
        CallMethod(".ctor", params, paramCt);
    }
}

ScriptMgr::tsAssembly::tsObject::~tsObject()
{
    //delete object_;
}

void ScriptMgr::tsAssembly::tsObject::CallMethod(string methodName, void** params, int paramCount)
{
    var method = mono_class_get_method_from_name(Class->GetClass(), methodName.c_str(), paramCount);

    if (method == nullptr)
    {
        return;
    }

    MonoObject* exception = nullptr;
    mono_runtime_invoke(method, object_, params, &exception);
}

void ScriptMgr::tsAssembly::tsObject::CallMethod(MonoMethodDesc* methodDesc, void** params)
{
    var method = mono_method_desc_search_in_class(methodDesc, Class->GetClass());

    if (method == nullptr)
    {
        return;
    }

    MonoObject* exception = nullptr;
    mono_runtime_invoke(method, object_, params, &exception);
}

void* ScriptMgr::tsAssembly::tsObject::GetPropertyValue(string name)
{

    var property = Class->GetProperty(name);

    if (property->accessibility & ~(u8)Accessibility::Public)
        return nullptr;

    void* val = nullptr;

    val = mono_property_get_value(property->property_, object_, nullptr, nullptr);

    return val;
}

void ScriptMgr::tsAssembly::tsObject::SetFieldValue(string name, void* value)
{
    var field = Class->GetField(name);


    /*
    if (field.accessibility & ~(u8)Accessibility::Public)
        return;
        */

    mono_field_set_value(object_, field->field_, value);
}

void ScriptMgr::tsAssembly::tsObject::SetPropertyValue(string name, void* value)
{
    var property = Class->GetProperty(name);

    /*
    if (property->accessibility & ~(u8)Accessibility::Public)
        return;
        */

    mono_property_set_value(property->property_, object_, &value, nullptr);
}

void ScriptMgr::tsAssembly::tsClass::SetStaticFieldValue(string name, void* value)
{
    var field = GetField(name);


    mono_field_static_set_value(mono_class_vtable(assembly->scriptMgr->s_RootDomain, GetClass()), field->field_, value);
}

void ScriptMgr::tsAssembly::tsObject::SetFieldValue(string name, string value)
{
    SetFieldValue(name, Class->assembly->scriptMgr->UTF8ToMonoString(value.c_str()));
}

void ScriptMgr::tsAssembly::tsObject::SetPropertyValue(string name, string value)
{
    SetPropertyValue(name, Class->assembly->scriptMgr->UTF8ToMonoString(value.c_str()));
}

uint8_t ScriptMgr::GetAccessibilityFlag(uint32_t flag)
{
    uint8_t accessibility = (uint8_t)Accessibility::None;

    switch (flag)
    {
    case MONO_FIELD_ATTR_PRIVATE:
    {
        accessibility = (uint8_t)Accessibility::Private;
        break;
    }
    case MONO_FIELD_ATTR_FAM_AND_ASSEM:
    {
        accessibility |= (uint8_t)Accessibility::Protected;
        accessibility |= (uint8_t)Accessibility::Internal;
        break;
    }
    case MONO_FIELD_ATTR_ASSEMBLY:
    {
        accessibility = (uint8_t)Accessibility::Internal;
        break;
    }
    case MONO_FIELD_ATTR_FAMILY:
    {
        accessibility = (uint8_t)Accessibility::Protected;
        break;
    }
    case MONO_FIELD_ATTR_FAM_OR_ASSEM:
    {
        accessibility |= (uint8_t)Accessibility::Private;
        accessibility |= (uint8_t)Accessibility::Protected;
        break;
    }
    case MONO_FIELD_ATTR_PUBLIC:
    {
        accessibility = (uint8_t)Accessibility::Public;
        break;
    }
    }

    return accessibility;
}

uint8_t ScriptMgr::GetPropertyAccessibility(MonoProperty* property)
{
    MonoMethod* propertyGetter = mono_property_get_get_method(property);
    uint32_t accessFlag = mono_method_get_flags(propertyGetter, nullptr) & MONO_METHOD_ATTR_ACCESS_MASK;

    var accessibility = GetAccessibilityFlag(accessFlag);

    // Get a reference to the property's setter method
    MonoMethod* propertySetter = mono_property_get_set_method(property);
    if (propertySetter != nullptr)
    {
        // Extract the access flags from the setters flags
        uint32_t accessFlag = mono_method_get_flags(propertySetter, nullptr) & MONO_METHOD_ATTR_ACCESS_MASK;
        if (accessFlag != MONO_FIELD_ATTR_PUBLIC)
            accessibility = (uint8_t)Accessibility::Private;
    }
    else
    {
        accessibility = (uint8_t)Accessibility::Private;
    }

    return accessibility;

}

uint8_t ScriptMgr::GetFieldAccessibility(MonoClassField* property)
{

    uint8_t accessibility = (uint8_t)Accessibility::None;
    uint32_t accessFlag = mono_field_get_flags(property) & MONO_FIELD_ATTR_FIELD_ACCESS_MASK;

    accessibility = GetAccessibilityFlag(accessFlag);

	return accessibility;
}

ScriptMgr::tsAssembly::tsClass* ScriptMgr::tsAssembly::GetClass(string space, string name)
{
	for (auto class_ : classes)
	{
        if (class_->name == name && class_->nameSpace == space)
            return class_;
	}

    if (scriptMgr && scriptMgr->mainAssembly) {
        for (auto class_ : scriptMgr->mainAssembly->classes)
        {
            if (class_->name == name && class_->nameSpace == space)
                return class_;
        }
    }

    return nullptr;
}

ScriptMgr::tsAssembly::tsClass* ScriptMgr::tsAssembly::GetClass(MonoClass* klass)
{

    for (auto class_ : classes)
    {
        if (class_->GetClass() == klass)
            return class_;
    }

    if (scriptMgr && scriptMgr->mainAssembly) {
        for (auto class_ : scriptMgr->mainAssembly->classes)
        {
            if (class_->GetClass() == klass)
                return class_;
        }
    }

	return nullptr;
}

int ScriptMgr::tsAssembly::GetMonoComponentIndex(string name)
{
	int i = 0;
	for (auto mono_component : monoComponents)
	{
		if (mono_component->name == name)
			return i;
		i++;
	}

	Logger::logger << "Cannot get mono component: "<<name<<std::endl;
				
	return -1;
}

ScriptMgr::tsAssembly::tsObject* ScriptMgr::tsAssembly::tsClass::CreateInstance(void** params, int paramCt)
{

    var obj = new tsObject(this, assembly, params, paramCt, true);

    //obj->CallMethod(".ctor", params, paramCt);

    return obj;

}

ScriptMgr::tsAssembly::tsClass::tsField::tsField(MonoClassField* field)
{
    field_ = field;

    name = mono_field_get_name(field);

    accessibility = ScriptMgr::GetFieldAccessibility(field);

}

ScriptMgr::tsAssembly::tsClass::tsProperty::tsProperty(MonoProperty* property)
{
    property_ = property;

    name = mono_property_get_name(property);

    accessibility = ScriptMgr::GetPropertyAccessibility(property);
}

MonoMethod* ScriptMgr::tsAssembly::tsClass::GetMethod(string name, int paramCount)
{
	for (auto method : methods)
	{
        var methodName = mono_method_get_name(method);

		if (strcmp(methodName, name.c_str()) == 0)
		{

            MonoMethodSignature* sig = mono_method_signature(method);

            int parCount = mono_signature_get_param_count(sig);

            if (parCount == paramCount)
            {
                return method;
            } else
            {
	            Logger::logger << "Found method: " << methodName << ", but invalid parameter count was found. (" << std::to_string(parCount) << ")" << std::endl;
            }

		}
	}

    return nullptr;
}

std::vector<ScriptMgr::tsAssembly::tsClass::tsField*> ScriptMgr::tsAssembly::tsClass::GetFields()
{
    var fieldsList = std::vector<tsField*>();

    void* itr = nullptr;

    while (MonoClassField* field = mono_class_get_fields(_class, &itr)) {

        fieldsList.push_back(new tsField(field));
    }

    if (parentTsClass) {
        var parentFields = parentTsClass->GetFields();

        fieldsList.insert(fieldsList.end(), parentFields.begin(), parentFields.end());
    }

    fields.clear();
    fields = fieldsList;

    return fieldsList;

}

std::vector<ScriptMgr::tsAssembly::tsClass::tsProperty*> ScriptMgr::tsAssembly::tsClass::GetProperties()
{
    var fieldsList = std::vector<tsProperty*>();

    void* itr = nullptr;

    while (MonoProperty* property = mono_class_get_properties(_class, &itr)) {

        fieldsList.push_back(new tsProperty(property));
    }

    if (parentTsClass) {
        var parentFields = parentTsClass->GetProperties();

        fieldsList.insert(fieldsList.end(), parentFields.begin(), parentFields.end());
    }

    properties.clear();
    properties = fieldsList;

    return fieldsList;
}

MonoObject* ScriptMgr::GetTransformValue(uint32_t id, int type)
{

    var engine = tmeGetCore();

    var transform = engine->GetActiveScene()->actorMgr->GetActor((int)id)->transform;

    var vec3Obj = engine->scriptMgr->mainAssembly->GetClass("Tomato", "Vector3");

    var sc = 1.f;

    void* p[] = {
    	&transform->position.x,
		&transform->position.y,
		&transform->position.z,
    };

    void* r[] = {
    &transform->rotation.x,
    &transform->rotation.y,
    &transform->rotation.z,
    };

    void* s[] = {
    &transform->scale.x,
    &transform->scale.y,
    &transform->scale.z,
    };

    switch (type)
    {
    case 0:
	    {
	        var obj = vec3Obj->CreateInstance(p, 3);

            /*
            var desc = mono_method_desc_new("TomatoScript.Tomato.Vector3::.ctor(single,single,single)", true);


            var method = mono_method_desc_search_in_class(desc, obj->GetClass()->GetClass());

            if (method == nullptr)
            {
                //return;
            }

            MonoObject* exception = nullptr;
            mono_runtime_invoke(method, obj->GetObject(), p, &exception);


	        //obj->CallMethod(desc, p);
	        */

	        return obj->GetObject();
	    }
    case 1:
        return vec3Obj->CreateInstance(r, 3)->GetObject();
    case 2:
        return vec3Obj->CreateInstance(s, 3)->GetObject();
    default:
        break;
    }

    return nullptr;
}

void ScriptMgr::SetTransformValue(uint32_t id, MonoObject* v, int type)
{
	//Logger::logger << mono_class_get_name(mono_object_get_class(v)) << std::endl;

    var engine = tmeGetCore();

    var actor = engine->GetActiveScene()->actorMgr->GetActor(static_cast<int>(id));

    var transform = actor->transform;

    var obj = new tsAssembly::tsObject(v, engine->scriptMgr->mainAssembly);

	var x = obj->GetFieldValue<float>("X");
    var y = obj->GetFieldValue<float>("Y");
    var z = obj->GetFieldValue<float>("Z");
    

    switch (type)
    {
    case 0:
        transform->position = { x,y,z };
        break;
    case 1:
        transform->rotation = { x,y,z };
        break;
    case 2:
        transform->scale = { x,y,z };
        break;
    default:
        break;
    }

    actor->transform = transform;

}

MonoClass* ScriptMgr::tsAssembly::GetMonoClass(string space, string name)
{
    MonoClass* klass = mono_class_from_name(image, space.c_str(), name.c_str());

    if (klass == nullptr)
        return nullptr;

    return klass;
}
