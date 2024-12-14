#ifndef MODEL_H
#define MODEL_H

#include "utils.hpp" 
#include "tomato/Render/Mesh.hpp"


namespace tmt::render {

struct Model
{
    std::vector<Mesh *> meshes;
    std::vector<int> materialIndices;

    Model(string path);
    Model(const aiScene *scene);
};

}

#endif