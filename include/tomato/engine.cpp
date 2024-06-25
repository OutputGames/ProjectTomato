#include "engine.hpp"

#include "ecs/component_registry.h"
#include "rendering/tmgl.h"
#include "scripting/script.h"

tmEngine* engine;


void tmeFullUpdate()
{

	tmPoll();
	//std::cout << "Cleared screen." << std::endl;
	engine->Update();
	//std::cout << "Updated engine." << std::endl;

	tmeUpdate();
	//std::cout << "Successfully updated." << std::endl;
}

extern "C" {
	void tmeInit(int width, int height, const char* name)
	{
		std::cout << "Initialized tomato." << std::endl;
		tmInit(width, height, name);
		std::cout << "Created window." << std::endl;
		engine = new tmEngine;
		std::cout << "Created engine core." << std::endl;
		engine->ScreenWidth = width;
		std::cout << "Assigned screen width" << std::endl;
		engine->ScreenHeight = height;
		std::cout << "Assigned screen height" << std::endl;

		engine->scriptMgr = new ScriptMgr;
		std::cout << "Created script engine." << std::endl;

		RegisterComponent(ComponentRegistry::RegisteredComponents());
		std::cout << "Registered components." << std::endl;
	}

	void tmeStartLoop()
	{
		while (!tmGetWindowClose())
		{
			tmeFullUpdate();
		}
	}

	void tmeLoad(const char* data)
	{
		tmeGetCore()->DeserializeGame(data);
	}
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

	applicationTime += 1.f;

	scriptMgr->Update();

	var currentScene = GetActiveScene();

	currentScene->OnUpdate();

	if (runtimePlaying)
	{
		currentScene->OnRuntimeUpdate();
	}



	renderMgr->Clear();


	tmCamera::GetMainCamera()->framebuffer->draw();
}

void tmEngine::Unload()
{
}

tmScene* tmEngine::GetActiveScene()
{
	//std::cout << "Getting active scene";
	//std::cout << loadedScenes.size();

	if (loadedScenes.size() <= 0) {
		loadedScenes.push_back(new tmScene());
		currentScene = 0;
	}

	return loadedScenes[currentScene];
}

string tmEngine::SerializeGame()
{
	nlohmann::json g;

	g["currentScene"] = currentScene;

	nlohmann::json s = nlohmann::json();

	for (auto loaded_scene : loadedScenes)
		s.push_back(loaded_scene->Serialize());

	g["scenes"] = s;

	g["renderMgr"] = renderMgr->SerializeGame();

	return g.dump(2);

}

void tmEngine::DeserializeGame(string data)
{
	nlohmann::json g = nlohmann::json::parse(data);

	currentScene = g["currentScene"];

	renderMgr->DeserializeGame(g["renderMgr"]);

	for (auto basic_jsons : g["scenes"])
		loadedScenes.push_back(new tmScene(basic_jsons));
		

}

tmEngine* tmeGetCore()
{
	return engine;
}
