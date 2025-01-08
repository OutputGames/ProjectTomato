#include "fs.hpp"

#include <ranges>

#include "globals.hpp"

using namespace tmt::fs;

ResourceManager* ResourceManager::pInstance;

ResourceManager::ResourceManager() { pInstance = this; }

void ResourceManager::ReloadShaders()
{
    for (auto shader : loaded_shaders | std::views::values)
    {
        shader->Reload();
    }
}
