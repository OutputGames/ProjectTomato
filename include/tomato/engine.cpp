#include "engine.hpp"

#include <thread>

#include "ecs/component_registry.h"
#include "rendering/tmgl.h"
#include "scripting/script.h"

tmEngine* engine;


void tmeFullUpdate()
{

	tmPoll();
	//Logger::logger << "Cleared screen." << std::endl;
	engine->Update();
	//Logger::logger << "Updated engine." << std::endl;

	engine->renderMgr->Clear();
	tmCamera::GetMainCamera()->framebuffer->draw();

	tmeUpdate();
	//Logger::logger << "Successfully updated." << std::endl;
}

extern "C" {
	void tmeInit(int width, int height, const char* name)
	{
		Logger::logger << "Initialized tomato." << std::endl;
		tmInit(width, height, name);
		Logger::logger << "Created window." << std::endl;
		engine = new tmEngine;
		Logger::logger << "Created engine core." << std::endl;
		engine->ScreenWidth = width;
		Logger::logger << "Assigned screen width" << std::endl;
		engine->ScreenHeight = height;
		Logger::logger << "Assigned screen height" << std::endl;

		engine->scriptMgr = new ScriptMgr;
		Logger::logger << "Created script engine." << std::endl;

		RegisterComponent(ComponentRegistry::RegisteredComponents());
		Logger::logger << "Registered components." << std::endl;

		std::atexit(tmeClose);
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
	tmInput::pollEvents();
}

void tmeClose()
{

	delete engine;

	tmClose();


	//std::this_thread::sleep_for(std::chrono::seconds(10));

	std::cout << "Unloaded engine.";
}

void tmEngine::Update()
{

	static float fpsFrameCount = 0;
	static float fps_last_time = 0;

	float currentFrame = glfwGetTime();
	tmTime::time++;
	fpsFrameCount++;
	tmTime::deltaTime = currentFrame - last_time;
	last_time = currentFrame;

	if (currentFrame - fps_last_time > 1.0f)
	{
		tmTime::fps = fpsFrameCount / (currentFrame - fps_last_time);
		fpsFrameCount = 0;
		fps_last_time = currentFrame;
	}

	var currentScene = GetActiveScene();

	currentScene->OnUpdate();

	if (runtimePlaying)
	{
		currentScene->OnRuntimeUpdate();
	}


	scriptMgr->Update();

}

void tmEngine::Unload()
{
	for (auto loaded_scene : loadedScenes) {
		if (runtimePlaying)
			loaded_scene->OnRuntimeUnload();
		loaded_scene->OnUnload();
		delete loaded_scene;
	}
	loadedScenes.clear();

	delete lighting;
	delete renderMgr;
	delete scriptMgr;

	for (auto loaded_prefab : tmPrefab::loaded_prefabs) delete loaded_prefab;
}

tmScene* tmEngine::GetActiveScene()
{
	//Logger::logger << "Getting active scene";
	//Logger::logger << loadedScenes.size();

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
	g["lightingMgr"] = lighting->Serialize();

	return g.dump(2);

}

void tmEngine::DeserializeGame(string data)
{
	nlohmann::json g = nlohmann::json::parse(data);

	currentScene = g["currentScene"];

	renderMgr->DeserializeGame(g["renderMgr"]);
	lighting->Deserialize(g["lightingMgr"]);

	for (auto basic_jsons : g["scenes"])
		loadedScenes.push_back(new tmScene(basic_jsons));
		

}

tmEngine* tmeGetCore()
{
	return engine;
}
