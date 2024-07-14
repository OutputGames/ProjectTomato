#if !defined(EDITOR_H)
#define EDITOR_H

#include "rendering/render.h"
#include "util/utils.h"

struct Asset;
struct ResourceManager;

class teEditorMgr {

public:

    teEditorMgr();

    struct teProject {
        string name, assetPath, libraryPath;
        ResourceManager* resMgr;

        teProject(string name);
    };

    struct teCamera : public tmBaseCamera
    {
        glm::vec3 position = { 0,0,10 }, rotation = { 0,-90,0 }, up = { 0,1,0 }, front = {0,0,1};

        void Update(glm::vec2 boundsMin, glm::vec2 boundsMax, bool block = false);

        glm::vec2 mousePosition = glm::vec2{ 0 }, lastMousePosition = glm::vec2{0};

        float moveSpeed = 0.25f;

    };

    void DrawUI();


    static inline teEditorMgr::teProject* currentProject = nullptr;

private:

    void _drawDockspace();
    void _drawMenuBar();
    void _drawSceneView();
    void _drawCameraView();
    void _drawDebug();
    void _drawLog();
    void _drawExplorer();
    void _drawProperties();
    void _drawAssetMenu();
    void _drawMaterialEditor();

    void onFileSelected(const Asset* asset);

    void _drawEntityTree(tmActor* actor);

    bool checkResize(int index = 0);

    teCamera* camera_ = new teCamera;

    std::vector<ImVec2> _windowSizes = std::vector<ImVec2>(2);

    tmFramebuffer* sceneFramebuffer;
    tmFramebuffer* gameFramebuffer;


    enum Icons
    {
	    PLAY,
        STOP,
        PAUSE
    };

    Dictionary<Icons, tmTexture*> icons = {};

    u32 _selected_actor = -1;
    Asset* selectedAsset = nullptr;
    bool _selectedActor = false, _selectedAsset = false;
    bool projectPopupOpen;
    bool do_reloading = false;

    int currentGizmoOperation = 7;

    vec2 transformPoint(float x, float y, vec2 boundMin, vec2 boundMax);
};

enum AssetType
{
    File = 0,
    Image = 1,
    Model = 2,
    Material = 3,
    Shader = 4,
    Script = 5,
    Animation = 6,
    Folder = 7,
    ScriptAssembly
};

enum flag_type
{
    INT = 0,
    FLOAT = 1,
    BOOL = 2,
    STRING = 3
};

struct Asset
{
    string path, fileName;
    AssetType type;
    std::filesystem::file_time_type lastWriteTime;

    union data
    {

    } data;

    union flag_uni
    {
        int int_0 ;
        float float_0;
        bool bool_0;
        const char* string_0;

        flag_uni() : int_0(0) {}
        flag_uni(int val) : int_0(val) {}
        flag_uni(float val) : float_0(val) {}
        flag_uni(bool val) : bool_0(val) {}
        flag_uni(const char* val) : string_0(val) {}
    };

    struct Flag
    {

        flag_type type;
        flag_uni value;

        Flag(int val) : type(INT), value(val) {}
        Flag(float val) : type(FLOAT), value(val) {}
        Flag(bool val) : type(BOOL), value(val) {}
        Flag(const char* val) : type(STRING), value(val) {}
        Flag() : type(INT), value(0) {}

    };
    
    Dictionary<string, Flag> flags;

    ImTextureID GetIcon();
};

struct ResourceManager
{
	Dictionary<AssetType, tmTexture*> icons = Dictionary<AssetType, tmTexture*>();

    ResourceManager(string assetPath);


    Asset* IsFileCached(string path);
    void ImportNewAsset(string path);
    void ReloadAsset(string path);
    void CreateNewAsset(string folder, AssetType type);


    void ReloadProjectAssembly();
    void RebuildScriptProject();
    void CheckEdits();
    void Update(bool check);
    static AssetType GetAssetTypeFromPath(string path);

    std::unordered_map<AssetType,Dictionary<string, Asset*>> cachedFiles = std::unordered_map<AssetType, Dictionary<string, Asset*>>{
        {File, {}},
        {Image, {}},
        {Model, {}},
    };

    List<fs::path> cachedFolders;

	Dictionary<string,tmModel*> cachedModels;
	Dictionary<string,tmTexture*> cachedTextures;
    Dictionary<string, int> cachedMaterials;
    fs::path currentDirectory;



private:
    string assetPath;

    tmShader* defaultShader;


};

inline void to_json(json& j, const Asset::Flag& f) {
    switch (f.type) {
    case INT:
        j = json{ {"type", "int"}, {"value", f.value.int_0} };
        break;
    case FLOAT:
        j = json{ {"type", "float"}, {"value", f.value.float_0} };
        break;
    case BOOL:
        j = json{ {"type", "bool"}, {"value", f.value.bool_0} };
        break;
    case STRING:
        j = json{ {"type", "string"}, {"value", f.value.string_0} };
        break;
    }
}

// Deserialization function
inline void from_json(const json& j, Asset::Flag& f) {
    std::string type = j.at("type").get<std::string>();
    if (type == "int") {
        f.type = INT;
        f.value.int_0 = j.at("value").get<int>();
    }
    else if (type == "float") {
        f.type = FLOAT;
        f.value.float_0 = j.at("value").get<float>();
    }
    else if (type == "bool") {
        f.type = BOOL;
        f.value.bool_0 = j.at("value").get<bool>();
    }
    else if (type == "string") {
        f.type = STRING;
        f.value.string_0 = j.at("value").get<std::string>().c_str();
    }
}

struct MaterialEditor
{
	enum NodeType
	{
		Add,
        Subtract,
        Multiply,
        Divide,
        Sine,
        Cosine,
        Number
	};

    enum InOutType
    {
	    Float
    };

    struct InoutAttribute
    {
        InOutType type;
        string name;

        InoutAttribute(InOutType t, string n);

        union data
        {
            int int_0;
            float float_0;
            bool bool_0;
            vec2 vec2_0;
            vec3 vec3_0;
            vec4 vec4_0;

            mat2 mat2_0;
            mat3 mat3_0;
            mat4 mat4_0;
        } data;
    };

    struct Node
    {
        NodeType type;

        Node(NodeType type);

        List<InoutAttribute*> In;
        List<InoutAttribute*> Out;

    };

    List<Node> nodes = List<Node>();
    List<std::pair<int, int>> links = List<std::pair<int, int>>();
    
    MaterialEditor();

};

#endif // EDITOR_H
