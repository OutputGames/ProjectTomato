#include "testproj.hpp"
#include <zstd.h>

Player::Player() : Object()
{
	var plr = tmt::obj::MeshObject::FromPrimitive(tmt::prim::Cube);
	plr->name = "PlayerMesh";
	plr->scale = { 1,2,1 };
	plr->position.y = -2;

	position = { -2,50,-2 };

	//plr->position.y = 1;
	plr->SetParent(this);
	plr->material->GetUniform("u_color")->v4 = tmt::render::Color::Red.getData();
	
	//var bfresTest = BfresLoader::Load("C:/Users/chris/OneDrive/Roms/splat3/1.1.1/Model/FldObj_DitchTree.bfres.zs");

	Mesh = plr;

	name = "Player";

	cam = tmt::obj::CameraObject::GetMainCamera();

	orientation = new tmt::obj::Object();
	orientation->SetParent(this);

	testForward = tmt::obj::MeshObject::FromPrimitive(tmt::prim::Cube);
	testForward->material->GetUniform("u_color")->v4 = tmt::render::Color::Green.getData();
	testForward->scale = glm::vec3{ 0.1 };

	testRight = tmt::obj::MeshObject::FromPrimitive(tmt::prim::Cube);
	testRight->material->GetUniform("u_color")->v4 = tmt::render::Color::Blue.getData();
	testRight->scale = glm::vec3{ 0.1 };

	testUp = tmt::obj::MeshObject::FromPrimitive(tmt::prim::Cube);
	testUp->material->GetUniform("u_color")->v4 = tmt::render::Color::Blue.getData();
	testUp->scale = glm::vec3{ 0.1 };

	var boxCol = new tmt::physics::ColliderObject(tmt::physics::ColliderInitInfo::ForCapsule(1,2));

	body = new tmt::physics::PhysicsBody(boxCol, 1);
	body->transRelation = tmt::physics::PhysicsBody::Parent;
	//body->constraint = tmt::physics::PhysicsBody::TransformConstraints::Rot;
	body->SetParent(this);

	emitter = new tmt::particle::ParticleEmitter();
	emitter->system->renderer.mesh = tmt::prim::GetPrimitive(tmt::prim::Sphere);
	emitter->rotation = { 90,0,0 };
	emitter->system->emission.rateOverTime = 0;
	emitter->system->collision.lifetimeLoss = 0.75f;
	emitter->system->collision.useColliders = true;


	//emitter->SetParent(plr);

	body->AddCollisionEvent([this](tmt::physics::Collision c)
		{
			//if (res0)
				//return;

			var map = c.other->GetObjectFromType<PaintableMap>();

			if (map)
			{


				map->GetPosition(c.contactPoint, c.faceId);
				if (!res0)
					bgfx::renderDocTriggerCapture();
				res0 = true;
			}
		});
}


void Player::Update()
{

	if (tmt::time::getTime() <= 1) {
		Object::Update();
		return;
	}

	float camRadius = 3;

	float camAdd = 0.01f;
	float horizontalInput = 0;
	float verticalInput = 0;
	float speed = Speed;

	if (tmt::input::Keyboard::GetKey(GLFW_KEY_LEFT) == tmt::input::Keyboard::Hold)
	{
		horizontalInput += 1;
	}
	if (tmt::input::Keyboard::GetKey(GLFW_KEY_RIGHT) == tmt::input::Keyboard::Hold)
	{
		horizontalInput -= 1;
	}
	if (tmt::input::Keyboard::GetKey(GLFW_KEY_UP) == tmt::input::Keyboard::Hold)
	{
		verticalInput += 1;
	}
	if (tmt::input::Keyboard::GetKey(GLFW_KEY_DOWN) == tmt::input::Keyboard::Hold)
	{
		verticalInput -= 1;
	}
	if (tmt::input::Keyboard::GetKey(GLFW_KEY_LEFT_SHIFT) == tmt::input::Keyboard::Hold)
	{
		speed = SwimSpeed;
		Mesh->scale.y = 0.1f;
		Mesh->position.y = -0.9f;
	} else
	{
		Mesh->scale.y = 2.0f;
		Mesh->position.y = -1.0f;
	}

	camAmt -= tmt::input::Mouse::GetMouseDelta().x * 0.01f;
	camY += tmt::input::Mouse::GetMouseDelta().y * 0.01f;

	camY = glm::clamp(camY, -1.25f, 1.f);

	auto global_position = Mesh->GetGlobalPosition();

	cam->position = global_position + glm::vec3{ glm::sin(camAmt) * camRadius, 2+camY, glm::cos(camAmt) * camRadius };
	//cam->LookAt/
	cam->LookAt(global_position + glm::vec3{ 0,1.0f,0 });

	// mama a third person camera behind u

	if (tmt::input::Mouse::GetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == tmt::input::Mouse::Hold)
	{
		if (shootTimer >= 1) {
			shootTimer = 0;
			emitter->Emit();

			var start = emitter->GetGlobalPosition();
			var end = emitter->GetGlobalPosition() + (emitter->GetUp()*(emitter->system->startSpeed/2));

			int subCount = 4;
			var itr = 1 / flt subCount;
			float val = 0;

			for (int i = 0; i < subCount; ++i)
			{

				var p = glm::lerp(start, end, val);
				var ray = tmt::physics::Ray{ p, glm::vec3{0,-1,0} };

				var hit = ray.Cast();

				if (hit && hit->hit)
				{
					var map = hit->hit->GetObjectFromType<PaintableMap>();
					if (map)
					{
						var dir = ray.direction;

						var right = glm::cross({ 0,1,0 }, dir);

						map->RenderPaintMap(map->CalculatePaintMatrix(hit->point, -hit->normal, right), dir);
					}
				}

				val += itr;
			}

		}
		shootTimer += tmt::time::getDeltaTime() *7.5f;
	} else
	{
		shootTimer = 0;
	}

	{

		var moveDir = glm::vec3{ -horizontalInput,0,verticalInput };

		var mag = tmt::math::magnitude(moveDir);

		var viewDir = position - glm::vec3{ cam->position.x, position.y,cam->position.z };
		orientation->SetForward(viewDir);


		if (mag >= 0.1 || tmt::input::Mouse::GetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == tmt::input::Mouse::Hold)
		{
			var cameraForward = glm::normalize(cam->camera->GetFront());

			if (verticalInput != 0)
				cameraForward *= verticalInput;

			float yaw = glm::degrees(atan2(cameraForward.x, cameraForward.z));

			var diff = rotation.y - yaw;

			//body->SetAngular({ 0,glm::radians(diff),0 });

			rotation.y = yaw;

		}

		if (mag >= 0.1)
		{

			moveDir = orientation->GetForward() * moveDir.z + orientation->GetRight() * moveDir.x;

			moveDir = glm::normalize(moveDir);

			var velo = moveDir * speed;
			//body->AddForce(velo * 10.0f);

			velo *= 0.25f;
			velo.y = body->GetVelocity().y;
			

			body->SetVelocity(velo);


 		}
	}
	
	var velo = body->GetVelocity();
	bool isGrounded = velo.y < 0.1 && velo.y > -0.1;

	if (tmt::input::Keyboard::GetKey(GLFW_KEY_SPACE) == tmt::input::Keyboard::Press && isGrounded ) {

 		body->AddImpulse(glm::vec3{ 0,5,0 });
	}

	var time = tmt::time::getTime();

	//SetForward(glm::vec3{ glm::sin(time*0.001), 0, glm::cos(time*0.001f) });

	//LookAt(glm::vec3{0,0,0});

	testForward->position = global_position + GetForward();
	testUp->position = global_position + GetUp();
	testRight->position = global_position + GetRight();

	tmt::debug::Gizmos::color = tmt::render::Color::Blue;
	//tmt::debug::Gizmos::DrawLine(global_position, global_position + (GetForward()*3.0f));

	//tmt::debug::Gizmos::color = tmt::render::Color::Red;
	//tmt::debug::Gizmos::DrawLine(global_position, global_position + (GetUp() * 3.0f));

	emitter->position = global_position + (GetForward()*1.5f) + glm::vec3{0,0.5f,0};
	emitter->rotation = rotation;
	emitter->rotation.x = 90;
	emitter->system->startSpeed = 15;
	emitter->system->collision.lifetimeLoss = 0.25f;
	emitter->system->collision.mass = 1.5;

	body->SetAngularFactor(glm::vec3{ 0,1,0 });
	body->SetDamping(0.75, 0.99f);

	Object::Update();
}

#define reads(name, type) var name = stream.Read<type>()


PaintableMap::PaintableMap(int paintMapSize) : Object()
{
	PaintMapSize = paintMapSize;
	paintMap = new tmt::render::RenderTexture(PaintMapSize, PaintMapSize, bgfx::TextureFormat::RGBA8, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);
	readBackMap = new tmt::render::RenderTexture(PaintMapSize, PaintMapSize, bgfx::TextureFormat::RGBA32F, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);

	paintShader = new tmt::render::Shader(tmt::render::ShaderInitInfo{new tmt::render::SubShader("paintMap/vert", tmt::render::SubShader::Vertex),new tmt::render::SubShader("paintMap/frag", tmt::render::SubShader::Fragment),});
	

	stain = new tmt::render::Texture("resources/textures/stains/EnemyDeath00.png");

	var matrix = GetTransform();
	//var data = InData{ matrix[0], matrix[1], matrix[2], matrix[3], glm::vec4(0) };

	bgfx::VertexLayout inDataLayout;
	inDataLayout.begin()
		.add(bgfx::Attrib::TexCoord0, 4, bgfx::AttribType::Float)
		.add(bgfx::Attrib::TexCoord1, 4, bgfx::AttribType::Float)
		.add(bgfx::Attrib::TexCoord2, 4, bgfx::AttribType::Float)
		.add(bgfx::Attrib::TexCoord3, 4, bgfx::AttribType::Float)
		.add(bgfx::Attrib::TexCoord4, 4, bgfx::AttribType::Float)
		.end();

	const bgfx::Memory* mem = 0;
	if (true)
	{
		std::vector<GLubyte> pixels(ReadbackDataSize * ReadbackDataSize * 4, (GLubyte)0xffffffff);
		mem = bgfx::copy(pixels.data(), ReadbackDataSize * ReadbackDataSize* 4);
	}
	//modelBuffer = bgfx::createDynamicVertexBuffer(bgfx::copy(&data, sizeof(InData)), inDataLayout, BGFX_BUFFER_COMPUTE_READ);
	colorBuffer = new tmt::render::Texture(ReadbackDataSize, ReadbackDataSize, bgfx::TextureFormat::RGBA32F, BGFX_TEXTURE_COMPUTE_WRITE | BGFX_SAMPLER_POINT);
	colorBufferRB = new tmt::render::Texture(colorBuffer->width, colorBuffer->height, bgfx::TextureFormat::RGBA32F,  BGFX_TEXTURE_READ_BACK | BGFX_TEXTURE_BLIT_DST | BGFX_SAMPLER_POINT);

	readBackShader = new tmt::render::ComputeShader(new tmt::render::SubShader("paintMap/readback-compute", tmt::render::SubShader::Compute));
}

void PaintableMap::Update()
{

	if (tmt::time::getTime() <= 1)
	{
		if (parent) {
			for (auto element : parent->children)
			{
				var msh = dynamic_cast<tmt::obj::MeshObject*>(element);

				if (msh)
				{
					Mesh = msh;

					Mesh->material->GetUniform("s_texColor")->tex = paintMap->realTexture;

					break;
				}
			}
		}

		if (!Mesh)
		{
			var msh = dynamic_cast<tmt::obj::MeshObject*>(parent);

			if (msh)
			{
				Mesh = msh;

				Mesh->material->GetUniform("s_texColor")->tex = paintMap->realTexture;
			}
		}


		body = GetObjectFromType<tmt::physics::PhysicsBody>();

		if (body)
		{
			body->AddParticleCollisionEvent([this](tmt::physics::ParticleCollision pc)
				{
					var dir = glm::normalize(pc.other->velocity);
					RenderPaintMap(CalculatePaintMatrix(pc.other->position, dir, pc.other->GetRight()), dir);
				});
		}
	}

	if (tmt::time::getTime() == 100.0f) {


		// {-0.557004273, 0, -0.739169538} r
		// {-0.734591067, -0.392369330, 0.553554237} d
		// {-3.08010530, 0.382091761, 2.32102656} p

		var paintBallPos = glm::vec3{ -3.08010530, 0.382091761, 2.32102656 };
		var direction = glm::vec3{ -0.734591067, -0.392369330, 0.553554237 };
		var right = glm::vec3{ -0.557004273, 0, -0.739169538 };

	}
}

void PaintableMap::RenderPaintMap(glm::mat4 spaceMatrix, glm::vec3 paintDir)
{
	var modelT = GetTransform();

	int viewId = readBackMap->vid;

	bgfx::setViewClear(viewId, BGFX_CLEAR_DEPTH);
	bgfx::touch(viewId);

	//paintShader->SetMat4("paintSpaceMatrix", spaceMatrix);
	//bgfx::setViewTransform(viewId, NULL, tmt::math::mat4ToArray(spaceMatrix));
	//bgfx::setTransform(tmt::math::mat4ToArray(modelT));
	paintShader->subShaders[0]->GetUniform("modelMatrix")->m4 = modelT;
	paintShader->subShaders[0]->GetUniform("paintSpaceMatrix")->m4 = spaceMatrix;
	paintShader->subShaders[1]->GetUniform("stainTex")->tex = stain;
	paintShader->subShaders[0]->GetUniform("paintBallDirection")->v4 = glm::vec4(paintDir, PaintMapSize);

	bgfx::setVertexBuffer(0, Mesh->mesh->vbh);
	bgfx::setIndexBuffer(Mesh->mesh->ibh);

	bgfx::setViewRect(viewId, 0, 0, PaintMapSize, PaintMapSize);

	bgfx::setImage(1, paintMap->realTexture->handle, 0, bgfx::Access::ReadWrite, paintMap->format);
	//bgfx::setBuffer(1, Mesh->mesh->vertexBuffer, bgfx::Access::Read);

	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);

	paintShader->Push(viewId);

	bgfx::setViewClear(viewId, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);
	bgfx::touch(viewId); // Clears the view (Color/Depth based on bgfx::reset flags)

}

glm::mat4 PaintableMap::CalculatePaintMatrix(glm::vec3 pos, glm::vec3 dir, glm::vec3 right)
{
	var paintBallPos = pos;
	dir = glm::vec3{ 0,-1,0 };
	right = { 1,0,0 };

	glm::vec3 localUp = glm::cross(right, dir);
	glm::mat4 paintViewMatrix = glm::lookAt(paintBallPos - dir * 2.0f,
		paintBallPos + dir, localUp);

	// Near and far plane: paint explodes with a radius of 1 and does not affect 
	// triangles more distant than 3 units.
	GLfloat nearPlane = 0.05f, farPlane = 3.0f;
	// Makes projection matrix.
	GLfloat frustumSize = randomFloat(1.25,1.25f);
	glm::mat4 paintProjectionMatrix = glm::ortho(-frustumSize, frustumSize, -frustumSize,
		frustumSize, nearPlane, farPlane);

	// Computes paint transform matrix.
	glm::mat4 paintSpaceMatrix = paintProjectionMatrix * paintViewMatrix;
	return paintSpaceMatrix;
}

glm::vec4 PaintableMap::GetPosition(glm::vec3 p, int faceId)
{
	//faceId = -1;
	if (faceId == -1)
		return glm::vec4{0};

	var matrix = GetTransform();
	//var data = InData{ matrix[0], matrix[1], matrix[2], matrix[3], glm::vec4(p,0) };


	bgfx::setBuffer(0, Mesh->mesh->vertexBuffers[0], bgfx::Access::Read);
	bgfx::setBuffer(1, Mesh->mesh->vertexBuffers[2], bgfx::Access::Read);
	bgfx::setImage(2, colorBuffer->handle,0, bgfx::Access::Write, colorBuffer->format);
	bgfx::setImage(3, paintMap->realTexture->handle, 0, bgfx::Access::ReadWrite, paintMap->format);
	bgfx::setBuffer(4, Mesh->mesh->ibh, bgfx::Access::Read);


	glm::vec4 objectPoint = glm::inverse(matrix) * glm::vec4(p, 1);
	objectPoint.w = flt faceId;

	tmt::debug::Gizmos::color = tmt::render::Color::Blue;
	tmt::debug::Gizmos::DrawSphere(glm::vec3((objectPoint)) + glm::vec3{0,2.5,0}, 0.05);
	//tmt::debug::Gizmos::DrawSphere(glm::vec3(p), 1);
	tmt::debug::Gizmos::DrawLine(p, p + glm::vec3{0, 5, 0});

	readBackShader->internalShader->GetUniform("u_collisionPoint")->v4 = objectPoint;
	readBackShader->Run(readBackMap->vid);

	return glm::vec4{ NAN };
}

Paintball::Paintball() : Object()
{
	var boxCol = new tmt::physics::ColliderObject(tmt::physics::ColliderInitInfo::ForSphere(0.5));

	body = new tmt::physics::PhysicsBody(boxCol, 1);
	body->transRelation = tmt::physics::PhysicsBody::Parent;
	//body->constraint = tmt::physics::PhysicsBody::TransformConstraints::Rot;
	body->SetParent(this);

	var plr = tmt::obj::MeshObject::FromPrimitive(tmt::prim::Sphere);
	plr->name = "BallMesh";
	plr->scale = glm::vec3{ 0.5 };

	plr->SetParent(this);
	plr->material->GetUniform("u_color")->v4 = tmt::render::Color::Blue.getData();

	body->AddCollisionEvent([this](tmt::physics::Collision collision)
	{


		var paintable = collision.other->GetObjectFromType<PaintableMap>();

		if (paintable && !exploded) {
			direction = body->GetVelocity();
			direction = glm::normalize(direction);
			paintable->RenderPaintMap(PaintableMap::CalculatePaintMatrix(position,direction, body->GetBasisColumn(0)), direction);
			if (epc == 0)
				bgfx::renderDocTriggerCapture();
			//exploded = true;
			epc++;
		}
	}
	);
}

void Paintball::Update()
{
	Object::Update();
	direction = body->GetVelocity();
	direction = glm::normalize(direction);
	body->SetDamping(0.75, 1);
}

template <typename T>
T ISSRead(std::istringstream& s)
{
	T header;
	s.read(reinterpret_cast<char*>(&header), sizeof(T));

	return header;
}

using BfresAttribFormat = BfresLoader::BfresAttribFormat;
#define IBinaryReader tmt::fs::BinaryReader&

glm::vec4 Read_10_10_10_2_SNorm(IBinaryReader reader)
{
	int value = reader.ReadInt32();
	return glm::vec4(
		(value << 22 >> 22) / 511.f,
		(value << 12 >> 22) / 511.f,
		(value << 2 >> 22) / 511.f,
		value >> 30);
}

float Read_8_UNorm(IBinaryReader reader) { return reader.ReadByte() / 255.f; };
float Read_8_Uint(IBinaryReader reader) { return reader.ReadByte(); };
float Read_8_Snorm(IBinaryReader reader) { return reader.ReadInt16() / 127.f; };
float Read_8_Sint(IBinaryReader reader) { return reader.ReadSByte(); };

float Read_16_UNorm(IBinaryReader reader) { return reader.ReadUInt16() / 65535.f; };
float Read_16_Uint(IBinaryReader reader) { return reader.ReadUInt16(); }
float Read_16_Snorm(IBinaryReader reader) { return reader.ReadSByte() / 32767.f; }
float Read_16_Sint(IBinaryReader reader) { return reader.ReadInt16(); }

glm::vec4 ReadAttribute(IBinaryReader reader, BfresLoader::BfresAttribFormat format)
{
	//TODO clean this up better

	switch (format)
	{
	case BfresAttribFormat::Format_10_10_10_2_SNorm: return Read_10_10_10_2_SNorm(reader);

	case BfresAttribFormat::Format_8_UNorm: return glm::vec4(Read_8_UNorm(reader), 0, 0, 0);
	case BfresAttribFormat::Format_8_UInt: return glm::vec4(Read_8_Uint(reader), 0, 0, 0);
	case BfresAttribFormat::Format_8_SNorm: return glm::vec4(Read_8_Snorm(reader), 0, 0, 0);
	case BfresAttribFormat::Format_8_SInt: return glm::vec4(Read_8_Sint(reader), 0, 0, 0);
	case BfresAttribFormat::Format_8_SIntToSingle: return glm::vec4(Read_8_Sint(reader), 0, 0, 0);
	case BfresAttribFormat::Format_8_UIntToSingle: return glm::vec4(Read_8_Uint(reader), 0, 0, 0);

	case BfresAttribFormat::Format_16_UNorm: return glm::vec4(Read_16_UNorm(reader), 0, 0, 0);
	case BfresAttribFormat::Format_16_UInt: return glm::vec4(Read_16_Uint(reader), 0, 0, 0);
	case BfresAttribFormat::Format_16_SNorm: return glm::vec4(Read_16_Snorm(reader), 0, 0, 0);
	case BfresAttribFormat::Format_16_SInt: return glm::vec4(Read_16_Sint(reader), 0, 0, 0);
	case BfresAttribFormat::Format_16_SIntToSingle: return glm::vec4(Read_16_Sint(reader), 0, 0, 0);
	case BfresAttribFormat::Format_16_UIntToSingle: return glm::vec4(Read_16_Uint(reader), 0, 0, 0);

	case BfresAttribFormat::Format_8_8_UNorm: return glm::vec4(Read_8_UNorm(reader), Read_8_UNorm(reader), 0, 0);
	case BfresAttribFormat::Format_8_8_UInt: return glm::vec4(Read_8_Uint(reader), Read_8_Uint(reader), 0, 0);
	case BfresAttribFormat::Format_8_8_SNorm: return glm::vec4(Read_8_Snorm(reader), Read_8_Snorm(reader), 0, 0);
	case BfresAttribFormat::Format_8_8_SInt: return glm::vec4(Read_8_Sint(reader), Read_8_Sint(reader), 0, 0);
	case BfresAttribFormat::Format_8_8_SIntToSingle: return glm::vec4(Read_8_Sint(reader), Read_8_Sint(reader), 0, 0);
	case BfresAttribFormat::Format_8_8_UIntToSingle: return glm::vec4(Read_8_Uint(reader), Read_8_UNorm(reader), 0, 0);

	case BfresAttribFormat::Format_8_8_8_8_UNorm: return glm::vec4(Read_8_UNorm(reader), Read_8_UNorm(reader), Read_8_UNorm(reader), Read_8_UNorm(reader));
	case BfresAttribFormat::Format_8_8_8_8_UInt: return glm::vec4(Read_8_Uint(reader), Read_8_Uint(reader), Read_8_Uint(reader), Read_8_Uint(reader));
	case BfresAttribFormat::Format_8_8_8_8_SNorm: return glm::vec4(Read_8_Snorm(reader), Read_8_Snorm(reader), Read_8_Snorm(reader), Read_8_Snorm(reader));
	case BfresAttribFormat::Format_8_8_8_8_SInt: return glm::vec4(Read_8_Sint(reader), Read_8_Sint(reader), Read_8_Sint(reader), Read_8_Sint(reader));
	case BfresAttribFormat::Format_8_8_8_8_SIntToSingle: return glm::vec4(Read_8_Sint(reader), Read_8_Sint(reader), Read_8_Sint(reader), Read_8_Sint(reader));
	case BfresAttribFormat::Format_8_8_8_8_UIntToSingle: return glm::vec4(Read_8_Uint(reader), Read_8_Uint(reader), Read_8_Uint(reader), Read_8_Uint(reader));

	case BfresAttribFormat::Format_16_16_UNorm: return glm::vec4(Read_16_UNorm(reader), Read_16_UNorm(reader), 0, 0);
	case BfresAttribFormat::Format_16_16_UInt: return glm::vec4(Read_16_Uint(reader), Read_16_Uint(reader), 0, 0);
	case BfresAttribFormat::Format_16_16_SNorm: return glm::vec4(Read_16_Snorm(reader), Read_16_Snorm(reader), 0, 0);
	case BfresAttribFormat::Format_16_16_SInt: return glm::vec4(Read_16_Sint(reader), Read_16_Sint(reader), 0, 0);
	case BfresAttribFormat::Format_16_16_SIntToSingle: return glm::vec4(Read_16_Sint(reader), Read_16_Sint(reader), 0, 0);
	case BfresAttribFormat::Format_16_16_UIntToSingle: return glm::vec4(Read_16_Uint(reader), Read_16_Uint(reader), 0, 0);

	case BfresAttribFormat::Format_16_16_Single: return glm::vec4(reader.ReadHalfFloat(), reader.ReadHalfFloat(), 0, 0);
	case BfresAttribFormat::Format_16_16_16_16_Single: return glm::vec4(reader.ReadHalfFloat(), reader.ReadHalfFloat(), reader.ReadHalfFloat(), reader.ReadHalfFloat());

	case BfresAttribFormat::Format_32_UInt: return glm::vec4(reader.ReadUInt32(), 0, 0, 0);
	case BfresAttribFormat::Format_32_SInt: return glm::vec4(reader.ReadInt32(), 0, 0, 0);
	case BfresAttribFormat::Format_32_Single: return glm::vec4(reader.ReadSingle(), 0, 0, 0);

	case BfresAttribFormat::Format_32_32_UInt: return glm::vec4(reader.ReadUInt32(), reader.ReadUInt32(), 0, 0);
	case BfresAttribFormat::Format_32_32_SInt: return glm::vec4(reader.ReadInt32(), reader.ReadInt32(), 0, 0);
	case BfresAttribFormat::Format_32_32_Single: return glm::vec4(reader.ReadSingle(), reader.ReadSingle(), 0, 0);

	case BfresAttribFormat::Format_32_32_32_UInt: return glm::vec4(reader.ReadUInt32(), reader.ReadUInt32(), reader.ReadUInt32(), 0);
	case BfresAttribFormat::Format_32_32_32_SInt: return glm::vec4(reader.ReadInt32(), reader.ReadInt32(), reader.ReadInt32(), 0);
	case BfresAttribFormat::Format_32_32_32_Single: return glm::vec4(reader.ReadSingle(), reader.ReadSingle(), reader.ReadSingle(), 0);

	case BfresAttribFormat::Format_32_32_32_32_UInt: return glm::vec4(reader.ReadUInt32(), reader.ReadUInt32(), reader.ReadUInt32(), reader.ReadUInt32());
	case BfresAttribFormat::Format_32_32_32_32_SInt: return glm::vec4(reader.ReadInt32(), reader.ReadInt32(), reader.ReadInt32(), reader.ReadInt32());
	case BfresAttribFormat::Format_32_32_32_32_Single: return glm::vec4(reader.ReadSingle(), reader.ReadSingle(), reader.ReadSingle(), reader.ReadSingle());
	}
	return glm::vec4{ 0 };
}

tmt::obj::ObjectLoader::SceneInfo BfresLoader::Load(string path)
{
	var sceneInfo = tmt::obj::ObjectLoader::SceneInfo{};

	std::ifstream compressedFile(path, std::ios::binary);
	std::vector<char> decompressedData;
	// Open the compressed file
	compressedFile.seekg(0, std::ios::end);
	size_t compressedSize = compressedFile.tellg();
	compressedFile.seekg(0, std::ios::beg);

	// Read the compressed file into a buffer
	std::vector<char> compressedBuffer(compressedSize);
	compressedFile.read(compressedBuffer.data(), compressedSize);

	// Determine the decompressed size (You should know or estimate it)
	size_t decompressedSize = ZSTD_getFrameContentSize(compressedBuffer.data(), compressedSize);
	if (decompressedSize == ZSTD_CONTENTSIZE_ERROR || decompressedSize == ZSTD_CONTENTSIZE_UNKNOWN) {
		std::cerr << "Error determining decompressed size!" << std::endl;
	}

	// Create a buffer for the decompressed data
	decompressedData.resize(decompressedSize);

	// Decompress the data
	size_t result = ZSTD_decompress(decompressedData.data(), decompressedSize, compressedBuffer.data(), compressedSize);
	if (ZSTD_isError(result)) {
		std::cerr << "Decompression failed: " << ZSTD_getErrorName(result) << std::endl;
	}

	//MemoryStreamBuf memoryBuf(decompressedData);

	var stream = tmt::fs::StringBinaryReader(std::string(decompressedData.begin(), decompressedData.end()));

	//static_assert(sizeof(BinaryHeader) == 0x10, "Size mismatch");

	var binHeader = stream.Read<BinaryHeader>();
	var header = stream.Read<BfresHeader>();

	string name = stream.fLoadString(header.NameOffset);

	stream.SeekBegin(header.MemoryPoolInfoOffset);
	var memoryPoolInfo = stream.Read<BufferMemoryPool>();

	static_assert(sizeof(FMDL) != 0x74);

	var mdls = stream.ReadArray<FMDL>(header.ModelOffset, header.ModelCount);

	for (var mdl : mdls)
	{
		var fvtx = stream.ReadArray<FVTX>(mdl.fvtxOffset, mdl.fvtxCount);
		for (var vtx : fvtx)
		{
			var attributes = stream.ReadArray<FVTXAttribute>(vtx.vertexAttributeArrayOffset, vtx.vertexAttributeCount);
			var bufferInfo = stream.ReadArray<FVTXBufferInfo>(vtx.vbuffInfoArrayOffset, vtx.bufferCount);
			var bufferStrides = stream.ReadArray<FVTXStrideInfo>(vtx.vbuffInfoStateOffset, vtx.bufferCount);

			std::vector<BufferData> bufferData;

			stream.SeekBegin((long)memoryPoolInfo.Offset + vtx.memPoolOff);
			for (int i = 0; i < vtx.bufferCount; i++)
			{
				stream.Align(8);
				BufferData buffer = {};
				buffer.data = stream.ReadArray<byte>(bufferInfo[i].Size);
				buffer.Stride = (ushort)bufferStrides[i].Stride;
				bufferData.push_back(buffer);
			}

			std::map<string, std::vector<glm::vec4>> v_data = std::map<string, std::vector<glm::vec4>>();

			for (var attr : attributes) {
				string attrName = stream.fLoadString(attr.nameOffset);

				std::vector<glm::vec4> data(vtx.vertexCount);

				var buffer = bufferData[attr.bufferIndex];
				MemoryStreamBuf buf(buffer.data);
				var memStream = tmt::fs::BinaryReader(&buf);

				BfresAttribFormat format = (BfresAttribFormat)attr.vbFormat;

				for (int i = 0; i < vtx.vertexCount; i++)
				{
					memStream.seekg(i * buffer.Stride + attr.bufferOffset, std::ios_base::_Seekbeg);
					data[i] = ReadAttribute(memStream,format);
				}

				v_data.insert(std::make_pair(attrName, data));
			}
		}
	}

	return sceneInfo;
}
