#include "actor.h"

#include "component_registry.h"
#include "engine.hpp"
#include "glm/gtx/quaternion.hpp"
//#include "glm/gtx/matrix_decompose.hpp"

using namespace nlohmann;

nlohmann::json tmActor::Serialize()
{
    json j;

    j["name"] = name;
    j["enabled"] = enabled;

    json t;

    glm::to_json(t["position"], transform->position);
    glm::to_json(t["rotation"], transform->rotation);
    glm::to_json(t["scale"], transform->scale);

    t["parent"] = transform->parentId;

    t["children"] = json();

    for (auto vector : transform->children.GetVector())
        t["children"].push_back(vector);

    j["transform"] = t;

    json c = json();

    for (auto component : components)
    {
        c[component->GetName()] = component->Serialize();
    }

    j["components"] = c;

    return j;
}

tmActor::tmActor(string name, tmActor* parent)
{
    this->name = name;

    tmeGetCore()->GetActiveScene()->actorMgr->InsertActor(this);

    transform->entityId = id;

    if (parent)
    {
        transform->SetParent(parent->transform);
    }
}

glm::mat4 tmActor::Transform::GetMatrix()
{
    glm::mat4 m(1.0);

    m = glm::translate(m, GetGlobalPosition());
    m *= glm::toMat4(glm::quat(GetGlobalRotation()));
    m = glm::scale(m, GetGlobalScale());

    return m;
}


void tmActor::Update()
{

}

std::shared_ptr<Component> tmActor::AttachComponent(string name)
{
    auto const& c = ComponentRegistry::cmp_map[name]();

    c->entityID = id;

    components.push_back(c);

    return c;
}

Component::Component()
{
	
}

void tmActor::Transform::CopyTransforms(glm::mat4 m)
{
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    //glm::decompose(m, scale, rotation, translation, skew, perspective);


}

glm::vec3 tmActor::Transform::GetGlobalPosition()
{
    var parent = GetParent();

    if (parent)
    {
        return (position * scale) + parent->GetGlobalPosition();
    }
    else
    {
        return position;
    }
}

glm::vec3 tmActor::Transform::GetGlobalRotation()
{
    var parent = GetParent();

    if (parent)
    {
        return rotation + parent->GetGlobalRotation();
    }
    else
    {
        return rotation;
    }
}

glm::vec3 tmActor::Transform::GetGlobalScale()
{
    var parent = GetParent();

    if (parent)
    {
        return scale * parent->GetGlobalScale();
    }
    else
    {
        return scale;
    }
}

void tmActor::Transform::SetParent(Transform* transform)
{
    if (transform) {
        transform->children.Add(entityId);
        parentId = transform->entityId;
    } else
    {
        var parent = GetParent();

        if (parent)
        {
            parent->children.Remove(entityId);
            parentId = -1;
        }
    }
}

tmActor::Transform* tmActor::Transform::GetParent()
{
	if (parentId != -1)
	{
        return tmeGetCore()->GetActiveScene()->actorMgr->GetActor(parentId)->transform;
	}

    return nullptr;
}

tmActor* Component::GetActor()
{
    return tmeGetCore()->GetActiveScene()->actorMgr->GetActor(entityID);
}

nlohmann::json Component::Serialize()
{
    json c{};

    c["enabled"] = enabled;

    return c;
}

void Component::Deserialize(nlohmann::json j)
{
    enabled = j["enabled"];
}

void tmActorMgr::InsertActor(tmActor *actor)
{
    actor->id = actorIndex.GetCount();
    actorIndex.Add(actor);
}

json tmActorMgr::Serialize()
{
    json a = json::array();

    for (tmActor* ac : actorIndex.GetVector())
    {
        a.push_back(ac->Serialize());
    }

    return a;
}

tmActor *tmActorMgr::GetActor(int id)
{
    return actorIndex[id];
}

tmScene::tmScene(nlohmann::json j)
{
    Deserialize(j.dump());
}

void tmScene::OnRuntimeStart()
{
}

void tmScene::OnRuntimeUpdate()
{
    for (auto vector : actorMgr->actorIndex.GetVector())
    {

        for (auto component : vector->components) {
            component->RuntimeUpdate();
        }
    }
}

void tmScene::OnRuntimeUnload()
{
}

void tmScene::OnStart()
{
}

void tmScene::OnUpdate()
{
    for (auto vector : actorMgr->actorIndex.GetVector())
    {

        for (auto component : vector->components) {
            if (component->value == "uninitialized")
            {
	            component->Start();
                if (tmeGetCore()->runtimePlaying)
                    component->RuntimeStart();
                component->value = "initialized";
            }
            component->Update();
        }
    }

    for (auto vector : actorMgr->actorIndex.GetVector())
    {

        for (auto component : vector->components) {
            component->LateUpdate();
        }
    }
}

void tmScene::OnUnload()
{
}

json tmScene::Serialize()
{
    json s{};

    s["actors"] = actorMgr->Serialize();

    return s;

}

void tmScene::Deserialize(string d)
{
    json s = json::parse(d);

    for (auto actorj : s["actors"])
    {
        var actor = new tmActor(actorj["name"]);
        actor->enabled = actorj["enabled"];

        glm::from_json(actorj["transform"]["position"], actor->transform->position);
        glm::from_json(actorj["transform"]["rotation"], actor->transform->rotation);
        glm::from_json(actorj["transform"]["scale"], actor->transform->scale);

        actor->transform->parentId = actorj["transform"]["parent"].get<int>();

        for (auto child : actorj["transform"]["children"])
            actor->transform->children.Add(child.get<int>());

        if (!actorj["components"].is_null()) {
            for (auto& [key, val] : actorj["components"].items())
            {
                var comp = actor->AttachComponent(key);
                comp->Deserialize(val);
            }
        }
    }
}

tmActor* tmActor::Transform::GetActor()
{
    return tmeGetCore()->GetActiveScene()->actorMgr->GetActor(entityId);
}
