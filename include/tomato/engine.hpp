#if !defined(ENGINE_HPP)
#define ENGINE_HPP

#include "ecs/actor.h"
#include "rendering/lighting.h"
#include "rendering/render.h"
#include "util/input.h"
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
	tmLighting* lighting = new tmLighting;
	ScriptMgr* scriptMgr;

	bool runtimePlaying = true;
	float ScreenWidth, ScreenHeight;

	void Update();
	void Unload();

	~tmEngine() { Unload(); }

	tmScene* GetActiveScene();

	struct tmTime
	{
		TMAPI inline static float time = 0;
		TMAPI inline static float deltaTime = 1.f / 60.f;
		TMAPI inline static float fps = 60.f;
	};

	string SerializeGame();
	void DeserializeGame(string data);

private:

	float last_time;

	std::vector<tmScene*> loadedScenes;

};
TMAPI tmEngine* tmeGetCore();


#endif // ENGINE_HPP
