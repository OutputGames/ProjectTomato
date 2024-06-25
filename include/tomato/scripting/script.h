#if !defined(SCRIPT_HPP)
#define SCRIPT_HPP

#include "util/utils.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

#include "engine.hpp"
#include "ecs/actor.h"
#include "mono/metadata/object.h"
#include "mono/metadata/mono-debug.h"
#include "mono/metadata/threads.h"
#include "mono/metadata/reflection.h"
#include "mono/metadata/attrdefs.h"
#include "mono/metadata/loader.h"
#include "mono/utils/mono-logger.h"

#define u8 uint8_t

struct TMAPI ScriptMgr {

	ScriptMgr();

	static bool CheckMonoError(MonoError& error)
	{
		bool hasError = !mono_error_ok(&error);
		if (hasError)
		{
			unsigned short errorCode = mono_error_get_error_code(&error);
			const char* errorMessage = mono_error_get_message(&error);
			printf("Mono Error!\n");
			printf("\tError Code: %hu\n", errorCode);
			printf("\tError Message: %s\n", errorMessage);
			mono_error_cleanup(&error);
		}
		return hasError;
	}

	static std::string MonoStringToUTF8(MonoString* monoString)
	{
		if (monoString == nullptr || mono_string_length(monoString) == 0)
			return "";

		MonoError error;
		char* utf8 = mono_string_to_utf8_checked(monoString, &error);
		if (CheckMonoError(error))
			return "";
		std::string result(utf8);
		mono_free(utf8);
		return result;
	}

	MonoString* UTF8ToMonoString(const char* s)
	{
		return mono_string_new(s_AppDomain, s);
	}

	enum class Accessibility : uint8_t
	{
		None = 0,
		Private = (1 << 0),
		Internal = (1 << 1),
		Protected = (1 << 2),
		Public = (1 << 3)
	};


	static uint8_t GetAccessibilityFlag(uint32_t flag);

	static uint8_t GetPropertyAccessibility(MonoProperty* property);

	static uint8_t GetFieldAccessibility(MonoClassField* field);

	struct TMAPI tsAssembly
	{
		tsAssembly(MonoAssembly* assembly);

		struct TMAPI tsAssemblyType
		{
		private:

			friend tsAssembly;

			tsAssembly* assembly = nullptr;
		};

		struct tsObject;

		struct TMAPI tsClass : tsAssemblyType
		{
			string nameSpace, name;

			tsClass(MonoClass* _class);
			tsClass(MonoImage* image,string space, string name) : tsClass(mono_class_from_name(image, space.c_str(), name.c_str())) {}

			MonoClass* GetClass()
			{
				return _class;
			}


			tsObject* CreateInstance(void** params=nullptr, int parameterCount=0);

			struct tsField
			{

				string name;

				uint8_t accessibility;

				tsField(MonoClassField* field);

			private:

				friend tsObject;
				friend tsClass;

				MonoClassField* field_;
			};
			struct tsProperty
			{

				string name;

				uint8_t accessibility;

				tsProperty(MonoProperty* property);

			private:

				friend tsObject;
				friend tsClass;

				MonoProperty* property_;
			};

			std::vector<MonoMethod*> methods;

			MonoMethod* GetMethod(string name,int paramCount = 0);

			std::vector<tsField*> fields = std::vector<tsField*>();
			std::vector<tsField*> GetFields();


			std::vector<tsProperty*> properties = std::vector<tsProperty*>();
			std::vector<tsProperty*> GetProperties();

			tsField* GetField(string name)
			{
				for (auto field : fields)
					if (field->name == name) return field;

				return nullptr;
			}

			tsProperty* GetProperty(string name)
			{
				for (auto field : properties)
					if (field->name == name) return field;

				return nullptr;
			}

			void SetStaticFieldValue(string name, void* value);

		private:

			friend tsAssembly;
			friend tsObject;

			MonoClass* parentClass;

			tsClass* parentTsClass;

			MonoClass* _class;
		};

		struct TMAPI tsObject
		{
			tsObject(tsClass* klass, tsAssembly* assembly, void** params = nullptr, int paramCt = 0, bool autoCallConstructor = true);
			tsObject(MonoObject* object, tsAssembly* assembly, void** params = nullptr, int paramCt = 0, bool autoCallConstructor = true);

			void CallMethod(string method, void** params = nullptr, int paramCt = 0);
			void CallMethod(MonoMethodDesc* method, void** params = nullptr);

			template <typename T>
			T GetFieldValue(string name)
			{

				var field = Class->GetField(name);


				if (field->accessibility & ~(u8)Accessibility::Public)
					return NULL;

				T val;

				mono_field_get_value(object_, mono_class_get_field_from_name(Class->GetClass(), name.c_str()), &val);

				return val;
			}

			void* GetPropertyValue(string name);

			void SetFieldValue(string name, void* value);
			void SetPropertyValue(string name, void* value);


			void SetFieldValue(string name, string value);
			void SetPropertyValue(string name, string value);

			MonoObject* GetObject() const
			{
				return object_;
			}

			tsClass* GetClass()
			{
				return Class;
			}

		private:

			tsClass* Class;
			MonoObject* object_;
			tsAssembly* _assembly;
		};

		tsClass* GetClass(string space, string name);
		tsClass* GetClass(MonoClass* klass);

		int GetMonoComponentIndex(string name);

	private:

		friend ScriptMgr;
		friend class MonoComponent;
		friend tsObject;

		MonoImage* image;
		MonoAssembly* assembly;
		ScriptMgr* scriptMgr;

		std::vector<string> types;
		std::vector<tsClass*> classes;
		std::vector<tsClass*> monoComponents;
		std::vector<tsClass*> components;

		MonoClass* GetMonoClass(string space, string name);

	};

	tsAssembly* LoadAssembly(string assemblyPath);



	void CompileFull();


	void Update();

	std::vector<tsAssembly*> assemblies;
	tsAssembly* mainAssembly;


	static MonoObject* GetTransformValue(uint32_t id, int type);
	static void SetTransformValue(uint32_t id, MonoObject* v, int type);

	MonoDomain* s_RootDomain;
	MonoDomain* s_AppDomain;

private:

};

class TMAPI MonoComponent : public Component
{
	CLASS_DECLARATION(MonoComponent)

public:
	MonoComponent(std::string&& initialValue) : Component(move(initialValue))
	{
	}

	void Deserialize(nlohmann::json j) override;
	MonoComponent(int componentIndex = -1);
	MonoComponent(const char* componentName) : MonoComponent(tmeGetCore()->scriptMgr->mainAssembly->GetMonoComponentIndex(componentName)) {}
	void Start() override;
	void Update() override;
	nlohmann::json Serialize() override;

private:

	ScriptMgr::tsAssembly::tsClass* class_;
	ScriptMgr::tsAssembly::tsObject* obj_;
	string className;

	void Initialize(int compIdx);
	void Initialize(const char* componentName) { Initialize(tmeGetCore()->scriptMgr->mainAssembly->GetMonoComponentIndex(componentName)); };


};

#endif // SCRIPT_HPP
