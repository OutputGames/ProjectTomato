#if !defined(ENGINE_HPP)
#define ENGINE_HPP

#include "ecs/actor.h"
#include "util/utils.h"


TMAPI void tmeInit(int width, int height, string name);
TMAPI void tmeUpdate();
TMAPI void tmeClose();

struct tmEngine
{
public:
	Scene* currentScene = new Scene;

	bool runtimePlaying = true;

	void Update();
	void Unload();
};
TMAPI tmEngine* tmeGetCore();


#endif // ENGINE_HPP
