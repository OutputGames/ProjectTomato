
#include "tomato/engine.hpp"
#include "tomato/rendering/render.h"
#include "tomato/rendering/tmgl.h"
#include "tomato/util/filesystem_tm.h"


int main(int argc, const char** argv) {


    tmeInit(800, 600, "TomatoEngine");

    auto shader = new tmShader(tmfs::loadFileString("resources/shaders/test/vertex.glsl"), tmfs::loadFileString("resources/shaders/test/fragment.glsl"));
    var texture = new tmTexture("resources/textures/test.png", true);
    var texture2 = new tmTexture("resources/textures/test3.png", true);

    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
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
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

    unsigned vao = tmgl::genVertexArray();
    unsigned vbo = tmgl::genBuffer(GL_ARRAY_BUFFER, vertices, sizeof(vertices), GL_STATIC_DRAW);
	tmgl::genVertexBuffer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float));
	tmgl::genVertexBuffer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    //unsigned ebo = tmgl::genBuffer(GL_ELEMENT_ARRAY_BUFFER, indices, sizeof(indices), GL_STATIC_DRAW);
    glBindVertexArray(0);

    tmEngine* engine = tmeGetCore();

    for (int i = 0; i < 10; ++i)
    {
        // calculate the model matrix for each object and pass it to shader before drawing
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, cubePositions[i]);
        float angle = 20.0f * i;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

        Actor* actor = new Actor("Cube" + std::to_string(i));
        actor->transform->position = cubePositions[i];
        actor->transform->rotation = (glm::vec3(1.0f, 0.3f, 0.5f) * angle);
    }

    Actor* cameraActor = new Actor("Camera");
    Camera* camera = cameraActor->AttachComponent<Camera>();

    while (!tmGetWindowClose())
    {
        cameraActor->transform->position = { 0,0,-3 };
        cameraActor->transform->rotation.y = 90;
        engine->Update();

        /*
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        */


        // create transformations
        // pass transformation matrices to the shader
        camera->UpdateShader(shader);

        shader->setInt("texture1", 0);
        shader->setInt("texture2", 1);


        texture->use(0);
        texture2->use(1);
        shader->use();
        glBindVertexArray(vao);// seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        //glDrawArrays(GL_TRIANGLES, 0, 3);

        for (auto const& actor : engine->currentScene->actorMgr->GetAllActors())
        {
            if (actor->GetComponent<Camera>())
                continue;

            glm::mat4 model = actor->transform->GetMatrix();

            shader->setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glBindVertexArray(0);

        tmeUpdate();
    }


    tmeClose();

    return 0;
}
