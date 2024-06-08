#include "engine.hpp"

#include "rendering/tmgl.h"

tmEngine* engine;

void tmeInit(int width, int height, string name)
{

	tmInit(width, height, name);
	engine = new tmEngine;
}

void tmeUpdate()
{
	tmSwap();
}

void tmeClose()
{
	tmClose();
}

void tmEngine::Update()
{
	currentScene->OnUpdate();

	if (runtimePlaying)
	{
		currentScene->OnRuntimeUpdate();
	}
}

void tmEngine::Unload()
{
}

tmEngine* tmeGetCore()
{
	return engine;
}
