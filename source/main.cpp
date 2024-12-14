

#include "testproject/testproj.hpp"

#include "tomato/tomato.hpp"

int main(int argc, const char** argv) {

	srand((unsigned)time(nullptr));

	var engine = tmt::engine::init();

	ddInit();

	//var model = tmt::obj::ObjectLoader::Load("resources/models/untitled.fbx");

	//var mesh = model->meshes[0];

	tmt::render::ShaderInitInfo info = {
		new tmt::render::SubShader("test/vert", tmt::render::SubShader::Vertex),
		new tmt::render::SubShader("test/frag", tmt::render::SubShader::Fragment),
	};


	var cam = new tmt::obj::CameraObject();

	var player = new Player();

	var shader = new tmt::render::Shader(info);

	var material = new tmt::render::Material(shader);

	var texture = new tmt::render::Texture("resources/textures/dcph.png");

	/*
	for (auto value : model.materials)
	{
		value->Reload(shader);
		value->state.cull = tmt::render::MaterialState::Counterclockwise;
		var color = value->GetUniform("s_texColor");
		color->tex = texture;
	}
	*/

	tmt::render::ShaderInitInfo sinfo = {
	new tmt::render::SubShader("sprite/vert", tmt::render::SubShader::Vertex),
	new tmt::render::SubShader("sprite/frag", tmt::render::SubShader::Fragment),
	};

	var spriteShader = new tmt::render::Shader(sinfo);

	var sprite = new tmt::ui::SpriteObject();
	sprite->material = new tmt::render::Material(spriteShader);

	var sprite2 = new tmt::ui::SpriteObject();
	sprite2->material = new tmt::render::Material(spriteShader);

	var button = new tmt::ui::ButtonObject();
	button->SetParent(sprite);

	/*
	button->AddClickEvent([model]()
		{
			model.root->active = !model.root->active;
		});
		*/

	var map = tmt::obj::MeshObject::FromPrimitive(tmt::render::prim::Cube);
	map->name = "Map";
	map->scale = { 50,1,50 };
	map->position.y = 0;

	var miniMap = tmt::obj::MeshObject::FromPrimitive(tmt::render::prim::Cube);
	miniMap->name = "MiniMap";
	miniMap->scale = { 1,0.01,1 };

	var paintable = new PaintableMap(512);
	paintable->name = "PaintableMap";
	paintable->SetParent(map);
	var paintBall = new Paintball();
	paintBall->position = { -3,2,-3 };

	miniMap->material->GetUniform("s_texColor")->tex = paintable->paintMap->realTexture;

	{
		var boxCol = new tmt::physics::ColliderObject(tmt::physics::ColliderInitInfo::ForMesh(map->mesh), map);

		var staticBody = new tmt::physics::PhysicsBody(boxCol, 0);
		staticBody->name = "MapBody";
		staticBody->transRelation = tmt::physics::PhysicsBody::Parent;
		staticBody->SetParent(map);
	}

	{
		/*
		var boxCol = new tmt::physics::ColliderObject(tmt::physics::ColliderInitInfo::ForSphere(1));

		var body = new tmt::physics::PhysicsBody(boxCol, 1);
		body->transRelation = tmt::physics::PhysicsBody::Parent;
		body->SetParent(model.root);
		*/
	}


	for (int x = 0; x < 2; x++)
	{
		for (int y = 0; y < 2; y++)
		{
			var mdl = tmt::obj::MeshObject::FromPrimitive(tmt::render::prim::Cube);
			mdl->material->GetUniform("u_color")->v4 = tmt::render::Color::Green.getData();

			mdl->position = { x * 1.1f,10,y*1.1f };

			var boxCol = new tmt::physics::ColliderObject(tmt::physics::ColliderInitInfo::ForBox({ 1,1,1 }));

			var body = new tmt::physics::PhysicsBody(boxCol, 1);
			body->transRelation = tmt::physics::PhysicsBody::Parent;
			body->SetParent(mdl);
		}
	}


	var ctr = 0;
	while (!glfwWindowShouldClose(engine->renderer->window)) {


		var mat = glm::mat4(1.0);
		//mat = glm::translate(mat, glm::vec3(glm::sin(ctr*0.01), 0, glm::cos(ctr * 0.01)));
		mat = glm::scale(mat, glm::vec3{0.5f});
		mat = glm::rotate(mat, glm::radians(-90.0f), glm::vec3{ 1.0f,0.f,0.f });

		//cam->position = glm::vec3{ 1,0.5,2 };

		//uniform->v4 = { flt ctr * 0.14, 0, 0, 0 };


		//model.root->scale = glm::vec3{ 0.0075f };

		miniMap->position.y = 3;

		//model.root->Update();

		sprite->scale = glm::vec3{ 100 };
		sprite2->scale = glm::vec3{ 100 };

		sprite->mainTexture = paintable->paintMap->realTexture;
		sprite2->mainTexture = paintable->colorBuffer;
		//sprite->mainColor = { glm::sin(ctr*0.01f)*0.5f+0.5f,glm::cos(ctr*0.01f)*0.5f+0.5f,1};

		var mousePos = tmt::input::Mouse::GetMousePosition();

		//sprite->position = { glm::sin(ctr * 0.01f)*100+250, glm::cos(ctr * 0.01f)*100+250,0 };
		sprite->position = { 100,100,0 };
		sprite2->position = { 100,210,0 };

		tmt::engine::update();

		ctr++;
	}
	tmt::render::shutdown();

    return 0;
}