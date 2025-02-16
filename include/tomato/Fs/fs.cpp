#include "fs.hpp"

#include <ranges>

#include "globals.hpp"

using namespace tmt::fs;

ResourceManager* ResourceManager::pInstance;

std::vector<u8> tmt::fs::readToBuffer(string path)
{
    // Open the file for reading
    std::ifstream fin(path);

    // Create an empty vector
    std::vector<u8> v;

    // Read the contents of the file and store them in the
    // vector
    char c;
    while (fin >> c)
    {
        v.push_back(c);
    }

    // Close the file
    fin.close();

    return v;
}

ResourceManager::ResourceManager() { pInstance = this; }

void ResourceManager::ReloadShaders()
{
    for (auto shader : loaded_shaders | std::views::values)
    {
        shader->Reload();
    }
}

TRES::TResFolder::TResFolder(BinaryReader* reader, TResFileBase* parent)
{
    type = TRS_Folder;
    name = reader->ReadString();
    this->parent = parent;

    var fileCount = reader->ReadInt32();

    for (int i = 0; i < fileCount; ++i)
    {
        var signature = reader->ReadString(4);

        if (signature == "TFIL")
            subfiles.push_back(new TResFile(reader, this));
        else if (signature == "TFLD")
            subfiles.push_back(new TResFolder(reader, this));
    }
}

TRES::TResFile::TResFile(BinaryReader* reader, TResFileBase* parent)
{
    name = reader->ReadString();
    type = TRS_File;
    this->parent = parent;
}

TRES::TRES(string path)
{
    var reader = new BinaryReader(path);

    if (!reader->CheckSignature("TRES"))
        return;

    var fileCount = reader->ReadInt32();

    for (int i = 0; i < fileCount; ++i)
    {
        var signature = reader->ReadString(4);

        if (signature == "TFIL")
            files.push_back(new TResFile(reader));
        else if (signature == "TFLD")
            files.push_back(new TResFolder(reader));
    }
}
