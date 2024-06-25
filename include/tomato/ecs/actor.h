#if !defined(ACTOR_H)
#define ACTOR_H

#include <memory>

#include "util/collections.h"
#include "util/utils.h"

#define TO_STRING( x ) #x

//****************
#define CLASS_DECLARATION( classname )                                                      \
public:                                                                                     \
    inline static const std::size_t Type = std::hash<const char*>()(TO_STRING( classname ));;                                                          \
    virtual bool IsClassType( const std::size_t classType ) const override;                 \
    virtual std::shared_ptr<Component> clone() const override                               \
	{                                                                                       \
    return std::make_shared<classname>(*this);                                              \
    }                                                                                       \

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

class Component;

class TMAPI tmActor {

    friend class tmActorMgr;

    nlohmann::json Serialize();

public:


    tmActor(string name, tmActor* parent = nullptr);

    class TMAPI Transform {
    public:
        glm::vec3 position = glm::vec3(0.0);
        glm::vec3 rotation = glm::vec3(0.0);
        glm::vec3 scale = glm::vec3(1.0);

        tmActor* GetActor();

        glm::mat4 GetMatrix();
        void CopyTransforms(glm::mat4 m);

        glm::vec3 GetGlobalPosition();
        glm::vec3 GetGlobalRotation();
        glm::vec3 GetGlobalScale();

        glm::vec3 TransformPoint(glm::vec3 p)
        {
            return glm::quat(rotation) * p;
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

    private:

        friend class tmScene;

        int parentId = -1;
        List<int> children;

        friend tmActor;

        int entityId = -1;
    };
    string name;
    uint32_t id;
    bool enabled;

    Transform* transform = new Transform;

    void Update();

    std::vector<std::shared_ptr<Component>> components;


    std::shared_ptr<Component> AttachComponent(string name);

    template< class ComponentType >
    ComponentType* GetComponent()
    {
        for (auto&& component : components) {
            if (component->IsClassType(ComponentType::Type))
                return static_cast<ComponentType*>(component.get());
        }

        return static_cast<ComponentType*>(nullptr);
    }

    template <class CompType, typename... Args>
    CompType* AttachComponent(Args&&... params)
    {

        CompType* c = GetComponent<CompType>();

        if (!GetComponent<CompType>()) {
            auto cc = components.emplace_back(std::make_shared< CompType >(std::forward< Args >(params)...));
            auto sc = static_cast<CompType*>(cc.get());;

            sc->entityID = id;

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

};

class TMAPI Component {
public:
    inline static const std::size_t                    Type = std::hash<const char*>()(TO_STRING(Component));;
    virtual bool                                IsClassType(const std::size_t classType) const {
        return classType == Type;
    }
    virtual std::shared_ptr<Component> clone() const
    {
        return std::make_shared<Component>(*this);
    }
public:

    virtual                                ~Component() = default;
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

    virtual void SetEnabled(bool enb)
    {
        enabled = enb;
    }

    // virtual void EngineRender();
   // virtual std::string PrintToJSON();
    //virtual void LoadFromJSON(nlohmann::json data);
    //virtual std::string GetIcon();

public:
    int entityID = -1;
    bool enabled = true;

    tmActor* GetActor();
    virtual nlohmann::json Serialize();
    virtual void Deserialize(nlohmann::json j);
};


class TMAPI tmActorMgr {
    List<tmActor*> actorIndex;

    friend tmActor;
    friend class tmScene;

    void InsertActor(tmActor* actor);

    nlohmann::json Serialize();

public:

    tmActor* GetActor(int id);

    auto GetAllActors()
    {
        return actorIndex.GetVector();
    }
};

class TMAPI tmScene
{
public:

    tmScene() = default;
    tmScene(nlohmann::json j);

    tmActorMgr* actorMgr = new tmActorMgr;

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
