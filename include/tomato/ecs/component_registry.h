#if !defined(COMP_REG)
#define COMP_REG

#include <memory>

#include "util/utils.h"
#include "actor.h"
#include "rendering/render.h"
#include "rendering/ui.h"

#include "scripting/script.h"

struct TMAPI ComponentRegistry
{
    template<typename T> static std::shared_ptr<Component> createInstance() { return std::make_shared< T >(); }

    //typedef std::map<std::string, Component* (*)()> map_type;
    inline static std::map<std::string, std::shared_ptr<Component>(*)()> cmp_map= std::map<std::string, std::shared_ptr<Component>(*)()>();;


    template <typename... C>
    struct ComponentRegister
    {

    };

    using RegisteredComponents = ComponentRegister<tmCamera, MonoComponent, tmMeshRenderer, tmLight, tmSkinnedMeshRenderer, tmAnimator, tmSkeleton, tmImage, tmText>;
};

template <typename ... C>
void RegisterComponent()
{
    ([]()
        {
            string rawname = typeid(C).name();

            string sub = "class ";

            std::string::size_type i = rawname.find(sub);

            if (i != std::string::npos)
                rawname.erase(i, sub.length());


            ComponentRegistry::cmp_map[rawname] = &ComponentRegistry::createInstance<C>;
        }(), ...);
}

template <typename ... C>
void RegisterComponent(ComponentRegistry::ComponentRegister<C...>)
{
    RegisterComponent<C ...>();
}


#endif // COMP_REG
