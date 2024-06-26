#if !defined(EDITOR_H)
#define EDITOR_H

#include "rendering/render.h"
#include "util/utils.h"

struct ResourceManager;

class teEditorMgr {

public:

    teEditorMgr();

    struct teProject {
        string name;
        ResourceManager* resMgr;
    };

    struct teCamera : public tmBaseCamera
    {
        glm::vec3 position, front, up;

        void Update();

    };

    void DrawUI();


    teEditorMgr::teProject* currentProject = nullptr;

private:

    void _drawDockspace();
    void _drawMenuBar();
    void _drawSceneView();
    void _drawGameView();
    void _drawDebug();
    void _drawLog();

    bool checkResize(int index = 0);

    teCamera* camera_ = new teCamera;

    std::vector<ImVec2> _windowSizes;

    tmFramebuffer* sceneFramebuffer;
    tmFramebuffer* gameFramebuffer;


    enum Icons
    {
	    PLAY,
        STOP,
        PAUSE
    };

    Dictionary<Icons, tmTexture*> icons = {};
};

struct ResourceManager
{
    void PackResources(string path, string pack_path);
};

#endif // EDITOR_H
