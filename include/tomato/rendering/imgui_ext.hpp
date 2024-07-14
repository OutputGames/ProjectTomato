#if !defined(IMGUI_EXT)
#define IMGUI_EXT

#include "util/utils.h"

#include "util/collections.h"

class tmTexture;

namespace ImGui {

    string TMAPI FilePicker(int type, string path, bool& found);
    TMAPI auto TexturePicker(string name, tmTexture* tex) -> tmTexture*;

    namespace InternalTM
    {
        void TMAPI SetFiles(std::unordered_map<int, std::vector<string>> files);
    }

}

#endif // IMGUI_EXT
