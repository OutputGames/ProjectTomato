

#include "tomato/tomato.hpp"

int main(int argc, const char** argv) {

	var engine = tmt::engine::init();

	tmt::render::Vertex vertices[] = {
		{ glm::vec3{ +0.5f,+0.5f,-0.5f }, {1,0,0}, {1,1} },
		{ glm::vec3{ +0.5f,-0.5f,-0.5f }, {0,1,0}, {1,0} },
		{ glm::vec3{ -0.5f,-0.5f,-0.5f }, {0,0,1}, {0,0} },
		{ glm::vec3{ -0.5f,+0.5f,-0.5f }, {1,1,1},{0,1} },

		{ glm::vec3{ +0.5f,+0.5f,0.5f }, {1,0,0}, {1,1} },
		{ glm::vec3{ +0.5f,-0.5f,0.5f }, {0,1,0},{1,0} },
		{ glm::vec3{ -0.5f,-0.5f,0.5f }, {0,0,1}, {0,0} },
		{ glm::vec3{ -0.5f,+0.5f,0.5f }, {1,1,1}, {0,1} },
	};

	u16 indices[] = {
		// back
		3,2,1,
		1,0,3,

		// front
		5,6,7,
		7,4,5,

		// left
		0,1,5,
		5,4,0,

		//right
		7,6,2,
		2,3,7,

		// bottom
		5,1,2,
		2,6,5,

		// top

		4,7,3,
		3,0,4

	};

	var mesh = tmt::render::createMesh(vertices, indices,arrsize(vertices), arrsize(indices), tmt::render::Vertex::getVertexLayout());



	tmt::render::ShaderInitInfo info = {
		new tmt::render::SubShader("vert", tmt::render::SubShader::Vertex),
		new tmt::render::SubShader("frag", tmt::render::SubShader::Fragment),
	};

	var shader = new tmt::render::Shader(info);

	var material = new tmt::render::Material(shader);
	var uniform = material->GetUniform("time");

	var texture = new tmt::render::Texture("resources/textures/dcph.png");
	var color = material->GetUniform("s_texColor");

	var obj = new tmt::obj::Object();
	var meshObj = new tmt::obj::MeshObject();
	meshObj->mesh = mesh;
	meshObj->material = material;

	meshObj->SetParent(obj);

	var ctr = 0;
	while (!glfwWindowShouldClose(engine->renderer->window)) {


		var mat = glm::mat4(1.0);
		//mat = glm::translate(mat, glm::vec3(glm::sin(ctr*0.01), 0, glm::cos(ctr * 0.01)));
		mat = glm::scale(mat, glm::vec3{ 1,1,1 });

		uniform->v4 = { flt ctr * 0.14, 0, 0, 0 };
		color->tex = texture;

		//mesh->draw((mat), material);

		obj->Update();

		tmt::render::update();

		ctr++;
	}
	tmt::render::shutdown();

    return 0;
}