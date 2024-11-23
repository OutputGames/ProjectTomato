#include "actor.h"

#include "component_registry.h"
#include "engine.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/quaternion.hpp"

#include "misc/phys.h"
//#include "glm/gtx/matrix_decompose.hpp"

using namespace nlohmann;

List<tmPrefab*> tmPrefab::loaded_prefabs;

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

    int i = 0;
    for (auto component : components)
    {
        i++;
        c[component->GetInternalName()+"||"+std::to_string(i)] = component->Serialize();
    }

    j["components"] = c;

    return j;
}

tmActor::tmActor()
= default;

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

    if (globalMat == mat4(0.0))
    {
        glm::mat4 m(1.0f);

        const glm::vec3 globalPosition = GetGlobalPosition();
        const glm::vec3 globalRotation = GetGlobalRotation();
        const glm::vec3 globalScale = GetGlobalScale();

        m = glm::translate(m, globalPosition);
        m *= glm::toMat4(glm::quat(glm::radians(globalRotation)));
        m = glm::scale(m, globalScale);

        globalMat = m;
    }

    return globalMat;
}


glm::mat4 tmActor::Transform::GetLocalMatrix()
{
    if (localMat == mat4(0.0)) {
        glm::mat4 m(1.0f);

        m = glm::translate(m, position);
        m *= glm::toMat4(glm::quat(glm::radians(rotation)));
        m = glm::scale(m, scale);

        localMat = m;
    }

    return localMat;
}


void tmActor::Update()
{

}

tmActor::~tmActor()
{
    delete transform;
}

std::shared_ptr<Component> tmActor::AttachComponent(string name)
{
    auto const& c = ComponentRegistry::cmp_map[name]();

    c->entityID = id;

    components.push_back(c);

    return c;
}

void tmActor::RemoveComponent(string n, int idx)
{
    int id = 0;
    int cid = 0;
    for (const auto& component : components)
    {

        if (component->GetName() == n)
        {
            if (cid == idx)
            {
                break;
            }
            cid++;
        }
        id++;
    }


    components.erase(components.begin() + id);
    componentCount--;
}

void tmActor::SetEnabled(bool b)
{
	enabled = b;

    for (auto component : components) component->SetEnabled(b);
}

bool tmActor::GetEnabled()
{
    if ((transform->parentId > -1))
        return enabled & tmActorMgr::GetSceneActor(transform->parentId)->GetEnabled();

    return enabled;
}

void tmActor::Delete()
{
    queuedForDelete = true;
}

void tmActor::DeleteRecursive()
{
    queuedForDeleteRecursive = true;
}


tmActor* tmActor::Duplicate(std::vector<tmActor*> actorList, int offset)
{
    var actor = new tmActor(name);

    actor->transform->position = transform->position;
    actor->transform->rotation = transform->rotation;
    actor->transform->scale = transform->scale;

    int ctr = 0;
    for (int child : transform->children.GetVector())
    {
        var c = actorList[child]->Duplicate(actorList, offset);
        c->transform->SetParent(actor->transform);
    }

    for (auto const& component : components)
    {
        std::shared_ptr<Component> new_component = component.get()->clone(offset);
        new_component->entityID = actor->id;
        actor->components.emplace_back(new_component);
    }

    return actor;
}

tmPrefab::tmPrefab()
{
	
}

tmPrefab::~tmPrefab()
{
    for (auto actor : actors)
        delete actor;
}

tmActor* tmPrefab::InsertActor(string name)
{

    var actor = new tmActor();
    actor->name = name;
    actor->id = actors.GetCount();
    actor->transform->entityId = actor->id;

    actors.Add(actor);

    return actor;
}

void tmPrefab::Insert()
{
	auto tm_actors = tmeGetCore()->GetActiveScene()->actorMgr;
    int offset = tm_actors->GetActorCount();

    /*
    var actors_ = List<tmActor>();

    for (auto vector : actors.GetVector())
    {
        actors_.Add(*vector);
    }

    for (int i = 0; i < actors.GetCount(); ++i)
    {
        var actor = actors_[i];

        int id = actor.id;
        tm_actors->InsertActor(&actor);

    	actor.id = id+offset;
        actor.transform->entityId = actor.id;
        
        for (int j = 0; j < actor.transform->children.GetCount(); ++j)
        {
            actor.transform->children[j] += offset;
        }

        if (actor.transform->parentId > -1) {
            actor.transform->parentId += offset;
        }
    }
    */

    for (auto actor : actors.GetVector())
    {
        if (actor->transform->parentId == -1) {
            actor->Duplicate(actors.GetVector(), offset);
        }
    }


}

Component::~Component()
{
	Unload();
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
    glm::decompose(m, scale, rotation, translation, skew, perspective);

    this->rotation = glm::eulerAngles(rotation) * RAD2DEG;
	this->scale = scale;
    this->position = translation;

    localMat = mat4(0.0);
    globalMat = mat4(0.0);


}

glm::vec3 tmActor::Transform::GetGlobalPosition()
{
    var parent = GetParent();

    if (parent)
    {
        return (position) + parent->GetGlobalPosition();
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

List<int> tmActor::Transform::GetRecursiveChildren()
{
    var childList = GetChildren();

    for (auto childIndex : children)
    {
        var child = tmeGetCore()->GetActiveScene()->actorMgr->GetActor(childIndex)->transform;
        childList.Add(child->GetRecursiveChildren());
    }

    return childList;
}

tmActor::Transform* tmActor::Transform::GetChild(string name)
{
    var actorList = tmeGetCore()->GetActiveScene()->actorMgr->GetAllActors();

    return GetChild(name, actorList);
}

tmActor::Transform* tmActor::Transform::GetChildContains(string name)
{
    var actorList = tmeGetCore()->GetActiveScene()->actorMgr->GetAllActors();

    return GetChildContains(name, actorList);

}

tmActor::Transform* tmActor::Transform::GetChild(string name, std::vector<tmActor*> actorList)
{

    for (auto childIndex : children.GetVector())
    {
        var child = actorList[childIndex];

        if (child->name == name)
            return child->transform;

    }

    for (auto childIndex : children.GetVector())
    {
        var child = actorList[childIndex];

        if (child->name == name)
            return child->transform;

        var found = child->transform->GetChild(name, actorList);
        if (found)
        {
            return found;
        }

    }

    return nullptr;
}

tmActor::Transform* tmActor::Transform::GetChildContains(string name, std::vector<tmActor*> actorList)
{

    for (auto childIndex : children.GetVector())
    {
        var child = actorList[childIndex];

        if (StringContains(child->name, name))
            return child->transform;

    }

    for (auto childIndex : children.GetVector())
    {
        var child = actorList[childIndex];

        if (StringContains(child->name, name))
            return child->transform;

        var found = child->transform->GetChildContains(name, actorList);
        if (found)
        {
            return found;
        }

    }

    return nullptr;
}

tmActor* Component::GetActor()
{
    return tmeGetCore()->GetActiveScene()->actorMgr->GetActor(entityID);
}

tmActor::Transform* Component::transform()
{
    return GetActor()->transform;
}

tmActor* Component::actor()
{
    return GetActor();
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
    if (id >= actorIndex.GetCount() || id < 0)
        return nullptr;

    return actorIndex[id];
}

tmActor* tmActorMgr::GetSceneActor(int id)
{
    return tmeGetCore()->GetActiveScene()->actorMgr->GetActor(id);
}

tmScene::tmScene()
{
    physicsMgr = new tmPhysicsMgr;
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

        if (!vector->GetEnabled())
            continue;

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
    for (tmActor* actor : actorMgr->actorIndex)
    {
        actor->transform->localMat = mat4(0.0);
        actor->transform->globalMat = mat4(0.0);
    }
        

    for (auto vector : actorMgr->actorIndex.GetVector())
    {

        if (!vector->GetEnabled())
            continue;

        if (vector->queuedForDelete)
        {

            for (auto actorid : vector->transform->children.GetVector())
            {
                var actor = tmeGetCore()->GetActiveScene()->actorMgr->GetActor(actorid);
                actor->transform->SetParent(nullptr);
            }

            tmeGetCore()->GetActiveScene()->actorMgr->actorIndex.Remove(vector);

            for (auto actor : actorMgr->actorIndex.GetVector())
            {
                if (actor->id > vector->id)
                {
                    actor->id--;
                }
            }


            delete vector;
            continue;
        }
        if (vector->queuedForDeleteRecursive)
        {
	        for (auto actorid : vector->transform->children.GetVector())
	        {
		        var actor = tmeGetCore()->GetActiveScene()->actorMgr->GetActor(actorid);
		        actor->DeleteRecursive();
	        }

	        tmeGetCore()->GetActiveScene()->actorMgr->actorIndex.Remove(vector);

            for (auto actor : actorMgr->actorIndex.GetVector())
            {
                if (actor->id > vector->id)
                {
                    actor->id--;
                }
            }

	        delete vector;
            continue;
        }

        if (vector->componentCount <= 0)
            continue;

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

    physicsMgr->Update();

    tmeGetCore()->lighting->Draw();
    

    for (auto vector : actorMgr->actorIndex.GetVector())
    {

        if (!vector->GetEnabled())
            continue;

        for (auto component : vector->components) {
            component->LateUpdate();
        }
    }
}

void tmScene::OnUnload()
{
    for (auto vector : actorMgr->actorIndex.GetVector())
    {
        for (auto component : vector->components) {
            component->Unload();
        }
    }
    delete actorMgr;
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

    actorMgr->actorIndex.Clear();

    for (auto actorj : s["actors"])
    {
        var actor = new tmActor(actorj["name"]);
        actor->SetEnabled(actorj["enabled"]);

        glm::from_json(actorj["transform"]["position"], actor->transform->position);
        glm::from_json(actorj["transform"]["rotation"], actor->transform->rotation);
        glm::from_json(actorj["transform"]["scale"], actor->transform->scale);

        actor->transform->parentId = actorj["transform"]["parent"].get<int>();

        for (auto child : actorj["transform"]["children"])
            actor->transform->children.Add(child.get<int>());

        if (!actorj["components"].is_null()) {
            for (auto& [key, val] : actorj["components"].items())
            {
                var cname = StringSplit(const_cast<std::string&>(key), "||")[0];
                var comp = actor->AttachComponent(cname);
                comp->Deserialize(val);
            }
        }
    }
}

tmActor* tmActor::Transform::GetActor()
{
    return tmeGetCore()->GetActiveScene()->actorMgr->GetActor(entityId);
}
