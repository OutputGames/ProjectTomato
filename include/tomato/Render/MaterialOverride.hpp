#ifndef MATERIALOVERRIDE_H
#define MATERIALOVERRIDE_H

#include "utils.hpp" 
#include "tomato/Render/Texture.hpp"


namespace tmt::render {

struct MaterialOverride
{
    std::string name;
    glm::vec4 v4 = glm::vec4(0);
    glm::mat3 m3 = glm::mat3(1.0);
    glm::mat4 m4 = glm::mat4(1.0);
    Texture *tex = nullptr;
};

}

#endif