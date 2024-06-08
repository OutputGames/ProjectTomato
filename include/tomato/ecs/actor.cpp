#include "actor.h"

#include "engine.hpp"
#include "glm/gtx/quaternion.hpp"
//#include "glm/gtx/matrix_decompose.hpp"

Actor::Actor(string name)
{
    this->name = name;

    tmeGetCore()->currentScene->actorMgr->InsertActor(this);

    transform->entityId = id;
}

glm::mat4 Actor::Transform::GetMatrix()
{
    glm::mat4 m(1.0);

    m = glm::translate(m, position);
    m *= glm::toMat4(glm::quat(rotation));
    m = glm::scale(m, scale);

    return m;
}


void Actor::Update()
{

}

void Actor::Transform::CopyTransforms(glm::mat4 m)
{
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    //glm::decompose(m, scale, rotation, translation, skew, perspective);


}

Actor* Component::GetActor()
{
    return tmeGetCore()->currentScene->actorMgr->GetActor(entityID);
}

void ActorMgr::InsertActor(Actor *actor)
{
    actor->id = actorIndex.GetCount();
    actorIndex.Add(actor);
}

Actor *ActorMgr::GetActor(int id)
{
    return actorIndex[id];
}

void Scene::OnRuntimeStart()
{
}

void Scene::OnRuntimeUpdate()
{
}

void Scene::OnRuntimeUnload()
{
}

void Scene::OnStart()
{
}

void Scene::OnUpdate()
{
    for (auto vector : actorMgr->actorIndex.GetVector())
    {

        for (auto component : vector->components) {
            if (component->value == "uninitialized")
            {
	            component->Start();
                component->value = "initialized";
            }
            component->Update();
        }
    }
}

void Scene::OnUnload()
{
}

Actor* Actor::Transform::GetActor()
{
    return tmeGetCore()->currentScene->actorMgr->GetActor(entityId);
}
