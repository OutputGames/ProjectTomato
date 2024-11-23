#if !defined(ACTOR_H)
#define ACTOR_H

#include <memory>

#include "util/collections.h"
#include "util/utils.h"
#include "icons/IconsFontAwesome6.h"

#define TO_STRING( x ) #x

//****************
#define CLASS_DECLARATION( classname )                                                      \
public:                                                                                     \
    inline static const std::size_t Type = std::hash<std::string>()(TO_STRING( classname ));;                                                          \
    virtual bool IsClassType( const std::size_t classType ) const override;                 \
    virtual std::shared_ptr<Component> clone(int offset) override                               \
	{                                                                                       \
    return std::make_shared<classname>(*this);                                              \
    }                                                                                       \

#define CLASS_DECLARATION_NO_CLONE( classname )                                                      \
public:                                                                                     \
    inline static const std::size_t Type = std::hash<std::string>()(TO_STRING( classname ));;                                                          \
    virtual bool IsClassType( const std::size_t classType ) const override;                 \

//****************
// CLASS_DEFINITION
// 
// This macro must be included in the class definition to properly initialize 
// variables used in type checking. Take special care to ensure that the 
// proper parentclass is indicated or the run-time type information will be
// incorrect. Only works on single-inheritance RTTI.
//****************
#define CLASS_DEFINITION( parentclass, childclass )                                         \
bool childclass::IsClassType( const std::size_t classType ) const {                         \
        if ( classType == childclass::Type )                                                \
            return true;                                                                    \
        return parentclass::IsClassType( classType );                                       \
}                                                                                           \


#define QUICKCLASS(parentclass, classname) \
class classname : public parentclass\
{\
    CLASS_DECLARATION(classname)\
public:\
    classname(std::string&& initialValue)\
        : parentclass(move(initialValue)) {\
    }\
    classname() = default; \

struct tmPhysicsMgr;
class MonoComponent;
class Component;

class TMAPI tmActor {

    friend class tmActorMgr;
    friend class tmPrefab;
    friend class tmScene;

    tmActor();

    nlohmann::json Serialize();

    bool enabled;

public:

    bool queuedForDelete = false, queuedForDeleteRecursive = false;


    tmActor(string name, tmActor* parent = nullptr);

    class TMAPI Transform {
    public:
        glm::vec3 position = glm::vec3(0.0);
        glm::vec3 rotation = glm::vec3(0.0);
        glm::vec3 scale = glm::vec3(1.0);

        tmActor* GetActor();

        glm::mat4 GetMatrix();
        glm::mat4 GetLocalMatrix();
        void CopyTransforms(glm::mat4 m);

        glm::vec3 GetGlobalPosition();
        glm::vec3 GetGlobalRotation();
        glm::vec3 GetGlobalScale();

        glm::vec3 TransformPoint(glm::vec3 p)
        {
            return glm::quat(glm::radians(GetGlobalRotation())) * p;
        }

        glm::vec3 LocalTransformPoint(glm::vec3 p)
        {
            return glm::quat(glm::radians(rotation)) * p;
        }

        glm::vec3 GetUp()
        {
            return TransformPoint({ 0,1,0 });
        }

        glm::vec3 GetForward()
        {
            return TransformPoint({ 0,0,1 });
        }

        glm::vec3 GetRight()
        {
            return TransformPoint({ 1,0,0 });
        }

        void SetParent(Transform* transform = nullptr);
        Transform* GetParent();

        List<int> GetChildren()
        {
            return children;
        }

        List<int> GetRecursiveChildren();

        Transform* GetChild(string name);
        Transform* GetChildContains(string name);

        Transform* GetChild(string name, std::vector<tmActor*> actorList);
        Transform* GetChildContains(string name, std::vector<tmActor*> actorList);

    private:

        friend class tmScene;
        friend tmPrefab;
        friend class tmModel;

        int parentId = -1;
        List<int> children;

        mat4 localMat, globalMat;

        friend tmActor;

        int entityId = -1;
    };
    string name;
    uint32_t id;
    u32 componentCount;

    Transform* transform = new Transform;

    void Update();
    ~tmActor();

    std::vector<std::shared_ptr<Component>> components;


    std::shared_ptr<Component> AttachComponent(string name);

    template< class ComponentType >
    ComponentType* GetComponent(int index = 0)
    {
	    //std::cout << "Searching for component: " << ComponentType::Type << "(" << typeid(ComponentType).name() << ")" <<  std::endl;
        List<ComponentType*> comps;
        for (auto&& component : components) {
            if (component->IsClassType(ComponentType::Type)) {
                //std::cout << "\tFound matching component: " << component->Type << std::endl;
                comps.Add(static_cast<ComponentType*>(component.get()));
            }
            else {
	            //std::cout << "\tComponent not matching: " << component->Type << std::endl;
            };
        }

        if (comps.GetCount() > 0)
            return comps[index];

        //std::cout << "Couldn't find a matching component: " << ComponentType::Type << std::endl;

        return static_cast<ComponentType*>(nullptr);
    }

    template< class ComponentType >
    List<ComponentType*> GetComponents()
    {
        List<ComponentType*> comps;
        for (auto& component : components) {
            if (component->IsClassType(ComponentType::Type)) {
                comps.Add(static_cast<ComponentType*>(component.get()));;
            }
            else {
            };
        }

        return comps;
    }

    template< class ComponentType >
    ComponentType* GetComponentInParent()
    {

        if (!transform->GetParent())
            return nullptr;

        var component = transform->GetParent()->GetActor()->GetComponent<ComponentType>();

        if (component)
        {
            return component;
        } else
        {
            return transform->GetParent()->GetActor()->GetComponentInParent<ComponentType>();
        }

        return static_cast<ComponentType*>(nullptr);
    }

    template <class CompType, typename... Args>
    CompType* AttachComponent(Args&&... params)
    {

        CompType* c = GetComponent<CompType>();

        if (c == nullptr || std::is_same<CompType, MonoComponent>().value)
        {
            auto cc = components.emplace_back(std::make_shared< CompType >(std::forward< Args >(params)...));
            auto sc = static_cast<CompType*>(cc.get());;

            sc->entityID = id;
            sc->componentIndex = GetComponents<CompType>().GetCount() - 1;
            componentCount++;

            return sc;

            /*
            for (auto&& component : components) {
                if (component->IsClassType(cc->Type)) {
                    CompType* comp = static_cast<CompType*>(component.get());
                    comp->entity = this;
                    c = comp;
                    return comp;
                } else
                {
                    cout << component->GetName() << " is not " << cc->GetName() << endl;
                }
            }
            */

        }

        return static_cast<CompType*>(c);
    }

    void RemoveComponent(string n, int idx = 0);

    void SetEnabled(bool b);

    bool GetEnabled();
    bool GetDirectEnabled()
    {
        return enabled;
    }

    void Delete();
    void DeleteRecursive();

    tmActor* Duplicate(std::vector<tmActor*> actorList, int offset = 0);

};

class TMAPI tmPrefab
{

public:

    tmPrefab();
    ~tmPrefab();

    string path;

    List<tmActor*> actors;

    tmActor* InsertActor(string name);

    void Insert();

    static List<tmPrefab*> loaded_prefabs;

};

class TMAPI Component {
public:
    inline static const std::size_t                    Type = std::hash<const char*>()(TO_STRING(Component));;
    virtual bool                                IsClassType(const std::size_t classType) const {
        return classType == Type;
    }

    virtual std::shared_ptr<Component> clone(int offset)
    {
        return std::make_shared<Component>(*this);
    }
public:

    virtual                                ~Component();
    Component(std::string&& initialValue)
        : value(initialValue) {
    }

    Component();

    virtual void imgui_properties() {};

    inline virtual std::string GetName() {

        std::string rawname = typeid(*this).name();

        std::string sub = "class ";

        std::string::size_type i = rawname.find(sub);

        if (i != std::string::npos)
            rawname.erase(i, sub.length());

        return rawname;
    }

    virtual std::string GetInternalName()
    {
        std::string rawname = typeid(*this).name();

        std::string sub = "class ";

        std::string::size_type i = rawname.find(sub);

        if (i != std::string::npos)
            rawname.erase(i, sub.length());

        return rawname;
    }

public:
    std::string                             value = "uninitialized";

    virtual void Start()
    {
    }

    virtual void Update()
    {
    }

    virtual void LateUpdate()
    {
    }

    virtual void Unload()
    {

    }

    virtual void RuntimeStart()
    {
	    
    }

    virtual void RuntimeUpdate()
    {
	    
    }

    virtual void SetEnabled(bool enb)
    {
        enabled = enb;
    }

    virtual void EngineRender()
    {
	    
    }

    // virtual void EngineRender();
   // virtual std::string PrintToJSON();
    //virtual void LoadFromJSON(nlohmann::json data);
    virtual std::string GetIcon()
    {
        return ICON_FA_GEARS;
    }

public:
    int entityID = -1;
    int componentIndex = 0;
    bool enabled = true;

    tmActor* GetActor();

    tmActor::Transform* transform();
    tmActor* actor();

    virtual nlohmann::json Serialize();
    virtual void Deserialize(nlohmann::json j);
};


class TMAPI tmActorMgr {
    List<tmActor*> actorIndex;

    friend tmActor;
    friend class tmScene;
    friend tmPrefab;

    void InsertActor(tmActor* actor);

    nlohmann::json Serialize();

public:

    tmActor* GetActor(int id);

    auto GetAllActors()
    {
        return actorIndex.GetVector();
    }

    auto GetActorCount() {
        return actorIndex.GetCount();
    }

    static tmActor* GetSceneActor(int id);
};

class TMAPI tmScene
{
public:

    tmScene();
    tmScene(nlohmann::json j);

    tmActorMgr* actorMgr = new tmActorMgr;
    tmPhysicsMgr* physicsMgr;

    void OnRuntimeStart();
    void OnRuntimeUpdate();
    void OnRuntimeUnload();

    void OnStart();
    void OnUpdate();
    void OnUnload();

    nlohmann::json Serialize();
    void Deserialize(string d);
};

#endif // ACTOR_H
