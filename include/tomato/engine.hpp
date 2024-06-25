#if !defined(ENGINE_HPP)
#define ENGINE_HPP

#include "ecs/actor.h"
#include "rendering/render.h"
#include "util/utils.h"


struct ScriptMgr;

extern "C" {
TMAPI void tmeInit(int width, int height, const char* name);
TMAPI void tmeStartLoop();
TMAPI void tmeLoad(const char* data);
}
TMAPI void tmeUpdate();
TMAPI void tmeClose();

struct TMAPI tmEngine
{
public:
	int currentScene = -1;
	tmRenderMgr* renderMgr = new tmRenderMgr;
	ScriptMgr* scriptMgr;

	bool runtimePlaying = true;
	float ScreenWidth, ScreenHeight;

	void Update();
	void Unload();

	tmScene* GetActiveScene();

	float applicationTime = 0;

	string SerializeGame();
	void DeserializeGame(string data);

private:

	std::vector<tmScene*> loadedScenes;

};
TMAPI tmEngine* tmeGetCore();


#endif // ENGINE_HPP
