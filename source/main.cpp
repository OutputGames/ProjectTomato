

#include "rendering/ui.h"

#include "scripting/script.h"
#include "tomato/engine.hpp"
#include "tomato/rendering/render.h"
#include "tomato/rendering/tmgl.h"
#include "tomato/util/filesystem_tm.h"
#include "tomatoEngine/editor.h"

tmCamera* camera;

int main(int argc, const char** argv) {

    tmfs::copyFile("scriptcore/bin/Debug/TomatoScript.dll", "scriptcore/TomatoScript.dll");
    tmeInit(1600, 1200, "TomatoEngine");
    ImGui::SetCurrentContext(tmGetCore()->ctx);

    tmEngine* engine = tmeGetCore();

    engine->lighting->SetCubemap(new tmCubemap("resources/textures/testhdr.png", true));

    engine->runtimePlaying = true;

    teEditorMgr* editor = new teEditorMgr;
	//editor->currentProject->resMgr->PackResources("out.rres", "resources/");

    fs::remove("game.tmg");

    if (tmfs::fileExists("game.tmg")) {
        tmeGetCore()->DeserializeGame(tmfs::loadFileString("game.tmg"));

        camera = tmCamera::GetMainCamera();

        fs::remove("game.tmg");

    }
    else {

        /*

        // world space positions of our cubes
        glm::vec3 cubePositions[] = {
            glm::vec3(0.0f,  0.0f,  0.0f),
            glm::vec3(2.0f,  5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f, -2.5f),
            glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3(2.4f, -0.4f, -3.5f),
            glm::vec3(-1.7f,  3.0f, -7.5f),
            glm::vec3(1.3f, -2.0f, -2.5f),
            glm::vec3(1.5f,  2.0f, -2.5f),
            glm::vec3(1.5f,  0.2f, -1.5f),
            glm::vec3(-1.3f,  1.0f, -1.5f)
        };

        tmShader* shader = tmShader::GetDefaultShader();

        var texture = new tmTexture("resources/models/godot_plush/godot_plush_basecolor.png", false);
        var texture3 = new tmTexture("resources/models/godot_plush/godot_plush_roughness.png", true);
        var texture4 = new tmTexture("resources/models/godot_plush/godot_plush_roughness.png", true);
        var texture5 = new tmTexture("resources/models/godot_plush/godot_plush_normal.png", true);
        var texture2 = new tmTexture("resources/textures/test3.png", true);

        tmMaterial* material = new tmMaterial(shader);
        material->SetTexture("albedo", texture);
        material->SetTexture("roughness", texture3);
        material->SetTexture("metallic", texture4);
        material->SetTexture("normal", texture5);
        material->SetField("metallic_override", 1.0f);
        material->SetField("roughness_override", 0.5f);


        tmModel* model = tmModel::LoadModel("resources/models/godot_plush.fbx");

        tmActor* map = new tmActor("Map");
        //map->AttachComponent<MonoComponent>("TestComponent");

        {
            tmActor* actor = new tmActor("Cube", map);
            actor->transform->position = glm::vec3{ 0 };
            actor->transform->rotation = { -90.f,0,0 };
            actor->transform->scale = glm::vec3{ 50 };

            var meshRenderer = actor->AttachComponent<tmMeshRenderer>(model, 0);
            meshRenderer->materialIndex = 0;
        }

        */

        tmActor* cameraActor = new tmActor("Camera");
        cameraActor->transform->position = { 0,0,10 };
        cameraActor->transform->rotation.y = -90;
        camera = cameraActor->AttachComponent<tmCamera>();
        camera->framebuffer = new tmFramebuffer(tmFramebuffer::Deferred, {tmeGetCore()->ScreenWidth, tmeGetCore()->ScreenHeight});
        //cameraActor->AttachComponent<MonoComponent>(scriptMgr->assemblies[0]->GetMonoComponentIndex("TestComponent"));

        var light = new tmActor("Directional Light");
        var pointLight = light->AttachComponent<tmLight>();
        pointLight->lightType = tmLight::Directional;
        pointLight->intensity = 5;
        light->transform->rotation = {-15,-30,0};

        /*
        var text = new tmActor("Text");
        var textRenderer = text->AttachComponent<tmText>();
        textRenderer->text = "Test Hi Hi";
        */

        tmfs::writeFileString("game.tmg", tmeGetCore()->SerializeGame());
        
    } 

    //scriptMgr->CompileFull();

    teEditorMgr::currentProject = new teEditorMgr::teProject("New Project");

    tmLighting::UnloadBRDF();

    while (!tmGetWindowClose())
    {
        tmPoll();

        engine->Update();

		editor->DrawUI();

        tmeUpdate();

    }

    return 0;
}