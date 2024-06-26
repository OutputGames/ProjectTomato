

#include "scripting/script.h"
#include "tomato/engine.hpp"
#include "tomato/rendering/render.h"
#include "tomato/rendering/tmgl.h"
#include "tomato/util/filesystem_tm.h"
#include "tomatoEngine/editor.h"

tmShader* shader;
tmCamera* camera;

int main(int argc, const char** argv) {

    tmfs::copyFile("scriptcore/bin/Debug/TomatoScript.dll", "scriptcore/TomatoScript.dll");
    tmeInit(800, 600, "TomatoEngine");


    ImGui::SetCurrentContext(tmGetCore()->ctx);

    tmEngine* engine = tmeGetCore();

    engine->runtimePlaying = false;

    teEditorMgr* editor = new teEditorMgr;
	//editor->currentProject->resMgr->PackResources("out.rres", "resources/");

    //fs::remove("game.tmg");

    if (tmfs::fileExists("game.tmg")) {
        tmeGetCore()->DeserializeGame(tmfs::loadFileString("game.tmg"));
    }
    else {

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

        shader = new tmShader(tmfs::loadFileString("resources/shaders/test/vertex.glsl"), tmfs::loadFileString("resources/shaders/test/fragment.glsl"));
        shader->setInt("texture1", 0);
        shader->setInt("texture2", 1);

        var texture = new tmTexture("resources/textures/test.png", true);
        var texture2 = new tmTexture("resources/textures/test3.png", true);

        tmMaterial* material = new tmMaterial(shader);
        material->textures.push_back(texture);

        tmMaterial* material2 = new tmMaterial(shader);
        material2->textures.push_back(texture2);


        tmModel* model = tmModel::LoadModel("resources/models/test.fbx");

        tmActor* map = new tmActor("Map");
        map->AttachComponent<MonoComponent>("TestComponent");
        for (int i = 0; i < 10; ++i)
        {
            float angle = 20.0f * i;

            tmActor* actor = new tmActor("Cube" + std::to_string(i), map);
            actor->transform->position = cubePositions[i];
            actor->transform->rotation = (glm::vec3(1.0f, 0.3f, 0.5f) * angle);

            var meshRenderer = actor->AttachComponent<tmMeshRenderer>(model, randval(0, 1));
            meshRenderer->materialIndex = randval(0, 1);

        }

        tmActor* cameraActor = new tmActor("Camera");
        cameraActor->transform->position = { 0,0,10 };
        cameraActor->transform->rotation.y = -90;
        camera = cameraActor->AttachComponent<tmCamera>();
        camera->framebuffer = new tmFramebuffer(tmFramebuffer::Deferred, {tmeGetCore()->ScreenWidth, tmeGetCore()->ScreenHeight});
        //cameraActor->AttachComponent<MonoComponent>(scriptMgr->assemblies[0]->GetMonoComponentIndex("TestComponent"));

    }

    glfwSetKeyCallback(tmGetCore()->window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
			if (key == GLFW_KEY_R && action == GLFW_PRESS)
			{
                shader->reload(tmfs::loadFileString("resources/shaders/test/vertex.glsl"), tmfs::loadFileString("resources/shaders/test/fragment.glsl"));
                camera->framebuffer->reload();
			}
        });
    tmfs::writeFileString("game.tmg", tmeGetCore()->SerializeGame());

    //scriptMgr->CompileFull();

    while (!tmGetWindowClose())
    {
        tmPoll();

        engine->Update();

		editor->DrawUI();

        tmeUpdate();

    }


    tmeClose();

    return 0;
}