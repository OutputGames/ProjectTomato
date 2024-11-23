#include "editor.h"

#include "engine.hpp"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui/misc/cpp/imgui_stdlib.h"
#include "icons/IconsFontAwesome6.h"
#include "scripting/script.h"

#define RRES_IMPLEMENTATION
#include "ImGuizmo.h"
#include "imnodes_internal.h"
#include "rres.h"

#include "ecs/component_registry.h"

#include "glm/ext/scalar_common.hpp"
#include "glm/gtx/dual_quaternion.hpp"

#include "misc/debug.h"

#include "rendering/imgui_ext.hpp"

#include "util/filesystem_tm.h"

#define DEBUG_DRAW_IMPLEMENTATION
#include "debug_draw.hpp"

#define imtogm(v) glm::vec2(v.x, v.y)
#define gmtoim(v) ImVec2(v.x, v.y)
#define checkext(ext) HasExtension(path, #ext)

teEditorMgr::teEditorMgr()
{
	camera_->position = { 0,0,0 };
	camera_->framebuffer = new tmFramebuffer(tmFramebuffer::Deferred, {1, 1});
	sceneFramebuffer = new tmFramebuffer(tmFramebuffer::Color, {1, 1});

	var io = ImGui::GetIO();

	io.Fonts->AddFontDefault();
	float baseFontSize = 16; // 13.0f is the size of the default font. Change to the font size you use.
	float iconFontSize = baseFontSize * 2.0f / 3.0f;
	// FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

	// merge in icons from Font Awesome
	static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.GlyphMinAdvanceX = iconFontSize;
	io.Fonts->AddFontFromFileTTF((std::string("resources/fonts/") + std::string(FONT_ICON_FILE_NAME_FAS)).c_str(),
	                             iconFontSize, &icons_config, icons_ranges);

	//ImGui::OpenPopup("FolderPopup");


}

void teEditorMgr::_drawDockspace()
{
	bool p_open = true;
	static bool opt_fullscreen = true; // Is the Dockspace full-screen?
	static bool opt_padding = false; // Is there padding (a blank space) between the window edge and the Dockspace?
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None; // Config flags for the Dockspace

	// In this example, we're embedding the Dockspace into an invisible parent window to make it more configurable.
	// We set ImGuiWindowFlags_NoDocking to make sure the parent isn't dockable into because this is handled by the Dockspace.
	//
	// ImGuiWindowFlags_MenuBar is to show a menu bar with config options. This isn't necessary to the functionality of a
	// Dockspace, but it is here to provide a way to change the configuration flags interactively.
	// You can remove the MenuBar flag if you don't want it in your app, but also remember to remove the code which actually
	// renders the menu bar, found at the end of this function.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

	// Is the example in Fullscreen mode?
	if (opt_fullscreen)
	{
		// If so, get the main viewport:
		const ImGuiViewport* viewport = ImGui::GetMainViewport();

		// Set the parent window's position, size, and viewport to match that of the main viewport. This is so the parent window
		// completely covers the main viewport, giving it a "full-screen" feel.
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		// Set the parent window's styles to match that of the main viewport:
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f); // No corner rounding on the window
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f); // No border around the window

		// Manipulate the window flags to make it inaccessible to the user (no titlebar, resize/move, or navigation)
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}
	else
	{
		// The example is not in Fullscreen mode (the parent window can be dragged around and resized), disable the
		// ImGuiDockNodeFlags_PassthruCentralNode flag.
		dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
	}

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
	// and handle the pass-thru hole, so the parent window should not have its own background:
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// If the padding option is disabled, set the parent window's padding size to 0 to effectively hide said padding.
	if (!opt_padding)
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	ImGui::Begin("DockSpace", &p_open, window_flags);

	// Remove the padding configuration - we pushed it, now we pop it:
	if (!opt_padding)
		ImGui::PopStyleVar();

	// Pop the two style rules set in Fullscreen mode - the corner rounding and the border size.
	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	// Check if Docking is enabled:
	ImGuiIO& io = ImGui::GetIO();

	// If it is, draw the Dockspace with the DockSpace() function.
	// The GetID() function is to give a unique identifier to the Dockspace - here, it's "MyDockSpace".
	ImGuiID dockspace_id = ImGui::GetID("EngineDS");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
}

void AlignForWidth(float width, float alignment = 0.5f)
{
	ImGuiStyle& style = ImGui::GetStyle();
	float avail = ImGui::GetContentRegionAvail().x;
	float off = (avail - width) * alignment;
	if (off > 0.0f)
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
}

void teEditorMgr::_drawMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New", "Ctrl+N"))
			{
				//ImguiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Create new project", nullptr, ".");
				Logger::logger << "made new project" << std::endl;
			}

			if (ImGui::MenuItem("Open", "Ctrl+O"))
			{
				Logger::logger << "opened new project" << std::endl;
			}

			if (ImGui::MenuItem("Save", "Ctrl+S"))
			{
				Logger::logger << "saved new project" << std::endl;
			}

			ImGui::EndMenu();
		}
	}
	ImGui::EndMainMenuBar();

	if (currentProject)
	{
		if (ImGui::BeginMenuBar())
		{
			AlignForWidth(ImGui::CalcTextSize(ICON_FA_PLAY "").x);
			bool p = ImGui::Button(ICON_FA_PLAY"");

			if (p)
			{
				tmeGetCore()->scriptMgr->CompileFull();
			}
		}
		ImGui::EndMenuBar();
	}
}

vec2 teEditorMgr::transformPoint(float x, float y, vec2 boundMin, vec2 boundMax)
{
	vec2 p = (vec2(x, y) * camera_->framebuffer->size);

	var tempCorner = (glm::clamp(p, boundMin, boundMax));

	return tempCorner;
}

void teEditorMgr::_drawSceneView()
{


	static glm::vec2 boundMin = glm::vec2(0), boundMax = glm::vec2(0);
	static bool mouseHovering = false;

	float gridSize = 500;

	float gridBounds = gridSize;

	tmglDebugRenderer::grid(gridSize, vec3(floorf(camera_->position.x/(gridBounds)), 0, floorf(camera_->position.z / (gridBounds)))*(gridBounds), 1.f, dd::colors::White);

	camera_->Update(boundMin, boundMax, !mouseHovering);


	sceneFramebuffer->use();
	camera_->framebuffer->draw();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	ImGui::Begin("Scene");
	{
		int xpos, ypos;
		glfwGetWindowPos(tmGetCore()->window, &xpos, &ypos);

		boundMin = imtogm(ImGui::GetCursorScreenPos());
		boundMax = boundMin + imtogm(ImGui::GetContentRegionAvail());
		ImDrawList* list = ImGui::GetWindowDrawList();

		ImVec2 cursorPos = ImGui::GetCursorPos();
		ImVec2 contentRegion = ImGui::GetContentRegionAvail();

		if (checkResize(0) == false)
		{
			ImGui::End();
			return;
		}

		unsigned tex = sceneFramebuffer->gAlb;
		//tex = tmeGetCore()->lighting->depthFBO->gDepth;

		ImGui::Image((ImTextureID)(tex), _windowSizes[0], ImVec2(0, 1), ImVec2(1, 0));


		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_IMP"))
			{
				string payload_n = *(const string*)payload->Data;

				var asset = currentProject->resMgr->IsFileCached(payload_n);

				if (asset)
				{
					switch (asset->type)
					{

					case Model:

						var model = currentProject->resMgr->cachedModels[asset->path];
						model->prefab->Insert();

						break;

					}
				}

			}
			ImGui::EndDragDropTarget();
		}

		mouseHovering = ImGui::IsItemHovered();


		if (tmInput::getMouseButton(TM_MOUSE_BUTTON_LEFT) == TM_PRESS && !ImGuizmo::IsOver() && !ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered())
		{
			glBindFramebuffer(GL_READ_FRAMEBUFFER, camera_->framebuffer->frameId);


			ImVec2 mouse_pos = ImGui::GetMousePos(); // Global mouse position
			ImVec2 window_pos = ImGui::GetWindowPos() + cursorPos; // Position of the current ImGui window

			// Calculate the relative mouse position
			ImVec2 relative_mouse_pos;
			relative_mouse_pos.x = mouse_pos.x - window_pos.x;
			relative_mouse_pos.y = mouse_pos.y - window_pos.y;

			vec2 mousePos = imtogm(relative_mouse_pos);

			glm::vec2 clampedMousePos = glm::clamp(mousePos, vec2(0, 0), imtogm(contentRegion));

			glReadBuffer(GL_COLOR_ATTACHMENT3);


			float pixel[4];
			glReadPixels(clampedMousePos.x, contentRegion.y - clampedMousePos.y, 1, 1, GL_RGBA, GL_FLOAT, pixel);

			glReadBuffer(GL_NONE);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

			int maxActors = tmeGetCore()->GetActiveScene()->actorMgr->GetActorCount();

			float index = pixel[0] * flt maxActors;
			index = glm::round(index);

			if (index < maxActors && index > 0)
			{
				_selectedActor = true;
				_selected_actor = index;
			}


			float v = pixel[0];

			//list->AddCircleFilled(gmtoim((tmInput::getMousePosition())), 20.f, ImColor(v,v,v));
		}


		//list->AddCircleFilled(gmtoim(transBoundMin), 5.f, ImColor(1.0, 0.0, 0.0f));
		//list->AddCircleFilled(gmtoim(transBoundMax), 5.f, ImColor(1.0, 0.0, 0.0f));

		ImGui::SetItemAllowOverlap();


		ImGuizmo::SetOrthographic(false);
		ImGuizmo::BeginFrame();
		ImGuizmo::Enable(true);

		ImGuizmo::SetDrawlist();

		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, _windowSizes[0].x, _windowSizes[0].y);

		glm::mat4 view = camera_->GetViewMatrix();
		glm::mat4 projection = camera_->GetProjectionMatrix();

		auto tm_actor = tmeGetCore()->GetActiveScene()->actorMgr->GetActor(_selected_actor);
		if (_selectedActor && tm_actor)
		{
			glm::mat4 matrix = tm_actor->transform->
			                                 GetLocalMatrix();

			if (tm_actor->GetComponent<tmImage>())
			{
				view = mat4(1.0);
				projection = mat4(1.0);
				projection = camera_->orthoProj;

			}

			ImGuizmo::Manipulate(&view[0][0], &projection[0][0],
			                     static_cast<ImGuizmo::OPERATION>(currentGizmoOperation), ImGuizmo::MODE::LOCAL,
			                     &matrix[0][0]);



			//tmglDebugRenderer::box(tm_actor->transform->GetGlobalPosition(), dd::colors::Orange, vec3(1, 1, 1));

			//ImGuizmo::DrawCubes(&view[0][0], &projection[0][0], &matrix[0][0], 1);

			if (ImGuizmo::IsUsing()) {

				tm_actor->transform->CopyTransforms(matrix);
			}
		} else
		{
			_selectedActor = false;
		}

		for (auto body : tmeGetCore()->GetActiveScene()->physicsMgr->bodies)
		{
			//tmglDebugRenderer::sphere((body->position), dd::colors::Green, 0.1f);
		}

		for (auto body : tmeGetCore()->GetActiveScene()->physicsMgr->colliders)
		{
			if (body->body) {
				if (body->type == Collider::Sphere)
				{
					tmglDebugRenderer::sphere((body->body->position), dd::colors::Red, dynamic_cast<SphereCollider*>(body)->radius);
				} else if (body->type == Collider::Box)
				{
					tmglDebugRenderer::box((body->body->position), dd::colors::Red, dynamic_cast<BoxCollider*>(body)->size);
				}
			}
		}

		bool show = true;

		if (show)
		{
			int indices[] = {
				ImGuizmo::TRANSLATE, ImGuizmo::ROTATE, ImGuizmo::SCALE, ImGuizmo::UNIVERSAL
			};

			int idxCt = arrsize(indices);
			idxCt = 3;

			const char* names[] = {
				ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT, ICON_FA_ROTATE, ICON_FA_MAXIMIZE, ICON_FA_GLOBE, ICON_FA_BOX
			};

			ImVec2 winSize = gmtoim(_windowSizes[0]);

			float windowBorder = 0.01f;
			ImVec2 windowSize = ImVec2(0.075, 0.25);

			ImVec2 rectStart = gmtoim( boundMin ) + (winSize * windowBorder);
			ImVec2 rectSize = ImVec2(winSize.x * windowSize.x, winSize.y * windowSize.y);

			float borderX = windowSize.x;
			float sizeX = rectSize.x - (rectSize.x * (borderX * (idxCt-1)));

			float borderY = windowSize.y * 0.2f;
			float sizeY = rectSize.y - (rectSize.y * (borderY * (idxCt-1)));

			ImVec2 picStart = rectStart + (rectSize * ImVec2(borderX, borderY));
			ImVec2 picSize = ImVec2(sizeX, sizeY) / ImVec2(1,arrsize(indices));

			ImVec2 sizeOffset = ImVec2(0, rectSize.y * (borderY * (idxCt-1)));

			list->AddRectFilled(rectStart, rectStart + rectSize + sizeOffset, ImColor(0.25f, 0.25, 0.25));
			//list->AddCircleFilled(picStart, 5.f, ImColor(1, 0, 0));
			//list->AddCircleFilled(rectStart, 5.f, ImColor(1, 0, 0));

			for (int i = 0; i < arrsize(indices); ++i)
			{
				ImGui::SetCursorScreenPos(picStart);
				if (ImGui::Button(names[i], picSize))
				{
					currentGizmoOperation = indices[i];
				}

				picStart.y += (picSize.y) + (rectSize.y * borderY);
			}

			//list->AddRectFilled(picStart, picStart + picSize, ImColor(1, 0, 0));
		}

#ifdef DEBUG

		var ctr = ImGui::GetContentRegionAvail();

		ImGui::SetCursorPos(ImGui::GetWindowContentRegionMin());

		float fps = tmEngine::tmTime::fps;

		ImGui::Text(("FPS: " + std::to_string(fps)).c_str());

#endif


	}
	ImGui::End();
}

void teEditorMgr::_drawCameraView()
{
}

void teEditorMgr::_drawDebug()
{
	ImGui::Begin("Debug Menu");

	ImGui::Text("Texture Debugger");

	static int selectedTex = 0;

	if (ImGui::Button("<"))
	{
		selectedTex--;
	}

	ImGui::SameLine();

	ImGui::Text(std::to_string(selectedTex).c_str());

	ImGui::SameLine();

	if (ImGui::Button(">"))
	{
		selectedTex++;
	}

	//selectedTex = ImClamp(selectedTex, 0, 100);

	ImGui::Image((ImTextureID)selectedTex, {256, 256});

	if (ImGui::Button("Reload shaders"))
	{
		List<tmShader*> compiledShaders;

		for (int i = 0; i < tmeGetCore()->renderMgr->materials.size(); ++i)
		{
			var material = tmeGetCore()->renderMgr->materials[i];
			material->Reload();
			compiledShaders.Add(material->shader);
		}

		for (auto shader : tmShader::shader_index.GetVector())
		{
			if (!compiledShaders.Contains(shader))
			{
				shader->reload(shader->vertData, shader->fragData, shader->isPath);
			}
		}

		Logger::logger << "Reloaded shaders!" << std::endl;

		//tmLighting::UnloadBRDF();
	}

	if (ImGui::Button("Reload scripts"))
	{
		currentProject->resMgr->RebuildScriptProject();
		currentProject->resMgr->ReloadProjectAssembly();

	}

	if (ImGui::Button("Reload models"))
	{
		for (auto cached_file : currentProject->resMgr->cachedFiles[Model])
		{
			currentProject->resMgr->ReloadAsset(cached_file.first);
		}
	}

	ImGui::SliderFloat("Camera Speed", &camera_->moveSpeed, EPSILON, 150.f);

	do_reloading = false;
	ImGui::Checkbox("Do asset reloading", &do_reloading);

	int fbt = camera_->framebuffer->framebuffer_type;

	ImGui::SliderInt("Framebuffer type", &fbt, 0, 3);

	camera_->framebuffer->framebuffer_type = (tmFramebuffer::FramebufferType)fbt;

	if (ImGui::Button("Recreate depth buffer"))
	{
		tmeGetCore()->lighting->depthFBO->recreate(vec2(1024));
	}

	if (ImGui::Button("Capture renderdoc frame"))
	{
		tmCapture();
	}

	ImGui::DragFloat("Line width", &tmglDebugRenderer::renderer->pointsize, 0.01f);

	ImGui::End();
}

void teEditorMgr::_drawLog()
{

	if (ImGui::Begin(ICON_FA_MESSAGE " Log")) {

		std::map<string, Logger::Level> loggedents;

		for (std::pair<std::pair<int, std::string>, Logger::Level> logged_entry : Logger::logger.loggedEntries)
		{
			if (!loggedents.count(logged_entry.first.second))
			{
				ImVec4 col = { 1, 1, 1, 1 };

				switch (logged_entry.second)
				{
				case Logger::INFO:
					break;
				case Logger::DBG:
					col = { 0, .5, 1, 1 };
					break;
				case Logger::WARN:
					col = { 1, 1, 0, 1 };
					break;
				case Logger::LOG_ERROR:
					col = { 1, .25, .25, 1 };
					break;
				}

				ImGui::TextColored(col, logged_entry.first.second.c_str());

				ImGui::NextColumn();

				loggedents.insert({ logged_entry.first.second, logged_entry.second });
			}
		}
	}

	ImGui::End();
}

void teEditorMgr::_drawEntityTree(tmActor* actor)
{
	if (actor == nullptr)
		return;

	const bool is_selected = (_selected_actor == actor->id);

	ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow;

	ImGuiTreeNodeFlags node_flags = base_flags;

	if (is_selected)
		node_flags |= ImGuiTreeNodeFlags_Selected;

	std::string cicon = ICON_FA_CHECK;

	if (!actor->GetEnabled())
		cicon = ICON_FA_XMARK;

	if (ImGui::Button((cicon + "###" + std::to_string(actor->id)).c_str()))
	{
		actor->SetEnabled(!actor->GetDirectEnabled());
	}


	ImGui::SameLine();

	string label = actor->name + "###" + std::to_string(actor->id);

	if (ImGui::TreeNodeEx(label.c_str(), node_flags))
	{
		if (is_selected)
			ImGui::SetItemDefaultFocus();

		if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
		{
			_selected_actor = actor->id;
			_selectedActor = true;
		}

		if (is_selected && ImGui::IsKeyPressed(ImGuiKey_Delete))
		{
			if (ImGui::IsKeyDown(ImGuiKey_RightShift))
			{
				actor->Delete();
			} else
			{
				actor->DeleteRecursive();
			}
		}

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
		{
			ImGui::Text(actor->name.c_str());
			ImGui::SetDragDropPayload("ENT_MOVE", &actor->id, sizeof(int));
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENT_MOVE"))
			{
				int payload_n = *(const int*)payload->Data;

				tmeGetCore()->GetActiveScene()->actorMgr->GetActor(payload_n)->transform->SetParent(actor->transform);
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::Indent();
		for (int child : actor->transform->GetChildren())
		{
			_drawEntityTree(tmeGetCore()->GetActiveScene()->actorMgr->GetActor(child)->transform->GetActor());
		}
		ImGui::Unindent();

		ImGui::TreePop();
	}
	else
	{
		// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
		if (is_selected)
		{
			ImGui::SetItemDefaultFocus();
		}
		if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
		{
			_selected_actor = actor->id;
			_selectedActor = true;
		}

		if (ImGui::BeginDragDropSource(
			ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoHoldToOpenOthers))
		{
			ImGui::Text(actor->name.c_str());
			ImGui::SetDragDropPayload("ENT_MOVE", &actor->id, sizeof(int));
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENT_MOVE"))
			{
				int payload_n = *(const int*)payload->Data;

				tmeGetCore()->GetActiveScene()->actorMgr->GetActor(payload_n)->transform->SetParent(actor->transform);
			}
			ImGui::EndDragDropTarget();
		}
	}
}

void teEditorMgr::_drawExplorer()
{
	ImGui::Begin(ICON_FA_MAGNIFYING_GLASS " Explorer");

	for (auto const& entity : tmeGetCore()->GetActiveScene()->actorMgr->GetAllActors())
	{

		if (entity->id >= tmeGetCore()->GetActiveScene()->actorMgr->GetActorCount() || entity->id < 0 || entity->queuedForDelete || entity->queuedForDeleteRecursive)
			continue;

		if (entity->transform->GetParent() == nullptr)
		{
			/*
			const bool is_selected = (Entity::selected_id == entity->id);

			std::string cicon = ICON_FA_CHECK;

			if (!entity->enabled)
			    cicon = ICON_FA_XMARK;

			if (ImGui::Button((cicon + "###" + to_string(entity->id)).c_str()))
			{
			    entity->enabled = !entity->enabled;
			}


			ImGui::SameLine();

			string label = entity->name + "" + to_string(entity->id);


			if (ImGui::Selectable(label.c_str(), is_selected)) {
			    Entity::selected_id = entity->id;
			    selectedEntity = true;
			}


*                */

			_drawEntityTree(entity);
		}
	}

	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
	{
		ImGui::OpenPopup("ent_create");
	}

	if (ImGui::BeginPopup("ent_create"))
	{


		Dictionary<string, int> create_menu = {
			{ "New Actor", 0 },

		};

		for (int i = 0; i < create_menu.GetCount(); ++i)
		{
			var button = create_menu.GetVector()[i];

			var assetType = button.second;;

			if (ImGui::Selectable(button.first.c_str())) {

				if (assetType == 0)
				{
					var actor = new tmActor("New Actor");
				}

				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}

void teEditorMgr::_drawProperties()
{
	ImGui::Begin("Properties");

	if (_selectedActor)
	{
		var actor = tmeGetCore()->GetActiveScene()->actorMgr->GetActor(_selected_actor);

		bool e = actor->GetDirectEnabled();

		ImGui::Checkbox("Enabled", &e);

		actor->SetEnabled(e);

		ImGui::SameLine();

		ImGui::InputText("Name", &actor->name);

		ImGui::Text(("Id: " + std::to_string(actor->id)).c_str());

		if (ImGui::CollapsingHeader("Transform"))
		{
			glm::mat4 matrix = actor->transform->GetLocalMatrix();

			float matrixTranslation[3], matrixRotation[3], matrixScale[3];
			ImGuizmo::DecomposeMatrixToComponents(&matrix[0][0], matrixTranslation, matrixRotation, matrixScale);

			ImGui::DragFloat3("Position", matrixTranslation);
			ImGui::DragFloat3("Rotation", matrixRotation);
			ImGui::DragFloat3("Scale", matrixScale);


			ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, &matrix[0][0]);

			actor->transform->CopyTransforms(matrix);

			if (ImGui::Button("Reset"))
			{
				actor->transform->position = vec3{0};
				actor->transform->rotation = vec3{0};
				actor->transform->scale = vec3{1};
			}
		}

		for (auto const& component : actor->components)
		{
			if (component == nullptr)
				continue;

			string name = component->GetName();

			if (ImGui::Button(ICON_FA_TRASH_CAN))
			{
				actor->RemoveComponent(name, component->componentIndex);
			}
			else
			{
				ImGui::SameLine();
				if (ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
				{
					component->EngineRender();
				}
			}
		}

		AlignForWidth(ImGui::CalcTextSize("Add Component").x + ImGui::GetStyle().ItemSpacing.x);

		if (ImGui::Button("Add Component"))
		{
			ImGui::OpenPopup("cmp_add");
		}

		if (ImGui::BeginPopup("cmp_add"))
		{

			static std::string filter = "";

			ImGui::InputText(ICON_FA_MAGNIFYING_GLASS, &filter);

			for (std::pair<const std::string, std::shared_ptr<Component>(*)()> cmp_map : ComponentRegistry::cmp_map)
			{

				if (cmp_map.first == "MonoComponent")
					continue;

				if (cmp_map.first.find(filter) != std::string::npos) {
					if (ImGui::Selectable(cmp_map.first.c_str()))
					{
						actor->AttachComponent(cmp_map.first);
						ImGui::CloseCurrentPopup();
						break;
					}
				}
			}

			var assemblies = tmeGetCore()->scriptMgr->assemblies;

			int i = 0;
			for (auto assembly :assemblies)
			{
				var components = assembly.second->GetComponents();

				for (auto component : components)
				{
					if (component->name == "MonoComponent")
						continue;

					if (component->name.find(filter) != std::string::npos) {
						if (ImGui::Selectable(component->name.c_str()))
						{
							var mono_cmp = actor->AttachComponent<MonoComponent>(component->name.c_str(), i);
							ImGui::CloseCurrentPopup();
							break;
						}
					}
				}
				i++;
			}

			ImGui::EndPopup();
		}

	} else if (_selectedAsset)
	{

		var tex = selectedAsset->GetIcon();

		float aspect = ImGui::GetWindowWidth() / ImGui::GetWindowHeight();
		var iconSize = ImVec2(250,250) * aspect;

		if (tex != (ImTextureID)-1)
		{
			ImGui::Image(tex, iconSize);
		} else
		{

			float padding = ImGui::GetStyle().FramePadding.x * 2.f;

			// Get the cursor position (top-left corner of the box)
			ImVec2 cursorPos = ImGui::GetCursorScreenPos();

			// Draw the box
			ImGui::GetWindowDrawList()->AddRect(
				cursorPos,
				ImVec2(cursorPos.x + iconSize.x, cursorPos.y + iconSize.y),
				IM_COL32(255, 255, 255, 255)
			);

			ImGui::SetCursorScreenPos(ImVec2(cursorPos.x + padding, cursorPos.y + padding));

			ImGui::TextUnformatted(selectedAsset->fileName.c_str());
		}

		ImGui::SameLine();

		ImGui::Text(selectedAsset->fileName.c_str());


		int i = 0;
		for (std::pair<std::string,Asset::Flag> flag : selectedAsset->flags.GetVector())
		{
			string name = flag.first;
			flag_type type = flag.second.type;

			switch (type)
			{
			case 0:
				{
				ImGui::DragInt(name.c_str(), &flag.second.value.int_0);
				}
				break;
			case 1:
				{
				ImGui::DragFloat(name.c_str(), &flag.second.value.float_0);
				}

				break;
			case  2:
				{
				ImGui::Checkbox(name.c_str(), &flag.second.value.bool_0);
				}

				break;
			}

			selectedAsset->flags[flag.first] = flag.second;

			i++;
		}

		if (ImGui::Button("Apply"))
		{
			string path = selectedAsset->path;
			currentProject->resMgr->ReloadAsset(selectedAsset->path);
			selectedAsset = currentProject->resMgr->IsFileCached(path);
		}

	}

	ImGui::End();
}

void teEditorMgr::onFileSelected(const Asset* asset)
{
	_selectedActor = false;
	_selectedAsset = true;
	selectedAsset = const_cast<Asset*>(asset);
}

void teEditorMgr::_drawAssetMenu()
{

	if (ImGui::Begin("Asset Menu")) {

		static fs::path currentDirectory = fs::path(currentProject->assetPath);

		if (ImGui::Button(ICON_FA_ARROW_UP))
			currentDirectory = currentDirectory.parent_path();

		ImGui::SameLine();

		ImGui::InputText("##dirup", &currentDirectory.string(), ImGuiInputTextFlags_ReadOnly);

		ImGui::Columns(7, 0, false);

		float aspect = ImGui::GetWindowWidth() / ImGui::GetWindowHeight();
		var iconSize = ImVec2(50, 50) * aspect;

		for (auto cached_folder : currentProject->resMgr->cachedFolders.GetVector())
		{

			var path = fs::path(cached_folder);
			if (path.parent_path() != currentDirectory)
				continue;
			var style = ImGui::GetStyle();

			auto user_texture_id = (ImTextureID)teEditorMgr::currentProject->resMgr->icons[AssetType::Folder]->id;
			ImGui::ImageButton(cached_folder.filename().string().c_str(), user_texture_id, iconSize - (style.FramePadding * 2.f));


			if (ImGui::BeginDragDropSource(
				ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoHoldToOpenOthers))
			{
				ImGui::Image(user_texture_id, iconSize - (style.FramePadding * 2.f));
				ImGui::SetDragDropPayload("ASSET_IMP", &cached_folder.string(), cached_folder.string().size() * sizeof(char));
				ImGui::EndDragDropSource();
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				currentDirectory /= cached_folder;
			}

			ImGui::SetNextItemWidth(iconSize.x - (style.WindowPadding.x * 2.f));
			ImGui::Text(cached_folder.filename().string().c_str());

			ImGui::NextColumn();
		}

		for (auto& [cachedType, files] : currentProject->resMgr->cachedFiles)
		{
			for (const auto& [cachedPath, cached_file] : files.GetVector())
			{
				var path = fs::path(cachedPath);
				if (path.parent_path() != currentDirectory)
					continue;

				ImTextureID id = ImTextureID(-1);

				id = cached_file->GetIcon();


				var style = ImGui::GetStyle();

				if (id == (ImTextureID)-1)
				{
					if (ImGui::Button(cached_file->fileName.c_str(), iconSize)) onFileSelected(cached_file);
				}
				else {
					if (ImGui::ImageButton(cached_file->fileName.c_str(), id, iconSize - (style.FramePadding * 2.f))) onFileSelected(cached_file);
				}


				if (ImGui::BeginDragDropSource(
					ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoHoldToOpenOthers))
				{
					ImGui::Image(id, iconSize - (style.FramePadding * 2.f));
					ImGui::SetDragDropPayload("ASSET_IMP", &cached_file->path, cached_file->path.size() * sizeof(char));
					ImGui::EndDragDropSource();
				}

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					if (is_directory(path))
						currentDirectory /= path.filename();

				}

				ImGui::SetNextItemWidth(iconSize.x - (style.WindowPadding.x * 2.f));
				ImGui::Text(cached_file->fileName.c_str());

				ImGui::NextColumn();
			}
		}


		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			ImGui::OpenPopup("asset_create");
		}

		if (ImGui::BeginPopup("asset_create"))
		{


			Dictionary<string, AssetType> create_menu = {
				{ "New Material", Material },
				{"New Script", Script}

			};

			for (int i = 0; i < create_menu.GetCount(); ++i)
			{
				var button = create_menu.GetVector()[i];

				var assetType = button.second;;

				if (ImGui::Selectable(button.first.c_str())) {

					currentProject->resMgr->CreateNewAsset(currentDirectory.lexically_relative(currentProject->assetPath).string(), assetType);

					ImGui::CloseCurrentPopup();
				}
			}



			ImGui::EndPopup();
		}
	}

	ImGui::End();
}

MaterialEditor::Node::Node(NodeType type)
{
	switch (type)
	{
	case Number:
		{
			In.Add(new InoutAttribute(Float, "Input"));
			Out.Add(new InoutAttribute(Float, "Value"));
		}
		break;
	}
}

void teEditorMgr::_drawMaterialEditor()
{
	static MaterialEditor* editor = new MaterialEditor;

	ImGui::Begin("Material Editor");

	{
		if (ImGui::IsKeyPressed(ImGuiKey_B))
		{
			editor->nodes.Add(MaterialEditor::Node(MaterialEditor::Number));
		}


		ImNodes::BeginNodeEditor();

		int i = 1;
		for (auto node : editor->nodes.GetVector())
		{
			ImNodes::BeginNode(i);
			ImGui::Dummy(ImVec2(80.0f, 45.0f));

			int si = 0;
			for (auto& attribute : node.In.GetVector())
			{
				const int output_attr_id = si + i;
				ImNodes::BeginInputAttribute(output_attr_id);

				switch (attribute->type)
				{
				case MaterialEditor::Float:
					{
						ImGui::SetNextItemWidth(100);
						ImGui::DragFloat(attribute->name.c_str(), &(attribute->data.float_0));
					}
					break;
				}

				ImNodes::EndInputAttribute();
				si++;
			}

			int isi = si;
			si = 0;
			for (auto attribute : node.Out.GetVector())
			{
				const int output_attr_id = si + i + isi;
				ImNodes::BeginOutputAttribute(output_attr_id);
				ImGui::Text(attribute->name.c_str());
				ImNodes::EndOutputAttribute();
				si++;
			}


			ImNodes::EndNode();
			i += 1;
			i += node.In.GetCount();
			i += node.Out.GetCount();
		}

		i = 0;
		for (std::pair<const int, int> p : editor->links.GetVector())
		{
			ImNodes::Link(i, p.first, p.second);
			i++;
		}

		ImNodes::MiniMap(0.2, ImNodesMiniMapLocation_BottomRight);

		ImNodes::EndNodeEditor();

		int start_attr = 0, end_attr = 0;
		if (ImNodes::IsLinkCreated(&start_attr, &end_attr))
		{
			editor->links.Add(std::make_pair(start_attr, end_attr));
		}

		if (ImGui::IsKeyPressed(ImGuiKey_V))
		{
			MaterialEditor::Node* outputNode;
		}
	}

	ImGui::End();
}


bool teEditorMgr::checkResize(int index)
{
	ImVec2 view = ImGui::GetContentRegionAvail();
	ImVec2 _windowSize = _windowSizes[index];

	if (view.x != _windowSize.x || view.y != _windowSize.y)
	{
		if (view.x == 0 || view.y == 0)
		{
			return false;
		}

		_windowSizes[index] = view;

		camera_->framebuffer->recreate({view.x, view.y});
		sceneFramebuffer->recreate({view.x, view.y});

		return true;
	}

	return true;
}

void CenteredText(const char* text)
{
	// Calculate the width of the text
	ImVec2 textSize = ImGui::CalcTextSize(text);

	// Get the available width
	float windowWidth = ImGui::GetContentRegionAvail().x;

	// Calculate the offset
	float textOffsetX = (windowWidth - textSize.x) * 0.5f;

	// Ensure the offset is not negative (i.e., the text is wider than the window)
	if (textOffsetX > 0.0f)
	{
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textOffsetX);
	}

	// Render the text
	ImGui::Text("%s", text);
}

ImTextureID Asset::GetIcon()
{
	if (teEditorMgr::currentProject->resMgr->icons.Contains(type))
		return (ImTextureID)teEditorMgr::currentProject->resMgr->icons[type]->id;
	else
	{
		switch (type)
		{
		case Image:
			return (ImTextureID)teEditorMgr::currentProject->resMgr->cachedTextures[path]->id;
		}
	}

	return (ImTextureID)-1;
}

ResourceManager::ResourceManager(string assetPath)
{
	currentDirectory = fs::path(teEditorMgr::currentProject->assetPath);
	CheckEdits();

	icons.Add(mkpair(Model, new tmTexture(teEditorMgr::currentProject->libraryPath + "Icons/solid/cubes.png", false,false)));
	icons.Add(mkpair(Folder, new tmTexture(teEditorMgr::currentProject->libraryPath + "Icons/solid/folder.png", false,false)));

	this->assetPath = assetPath;

	RebuildScriptProject();
	ReloadProjectAssembly();
}

Asset* ResourceManager::IsFileCached(string path)
{
	std::replace(path.begin(), path.end(), '/', '\\');

	var type = GetAssetTypeFromPath(path);

	if (cachedFiles[type].Contains(path))
		return cachedFiles[type][path];

	return nullptr;
}

void ResourceManager::ImportNewAsset(string path)
{

	std::replace(path.begin(), path.end(), '/', '\\');
	//Asset::AssetType type = Asset::GetTypeFromExtension(path);
	//Asset* asset = assetImportFuncs[type](this, path);

	var type = GetAssetTypeFromPath(path);
	Dictionary<string, Asset::Flag> flags = {
		{"Flip", Asset::Flag(false)},
		{"TextureType", Asset::Flag(0)},
		{"Model Scale", Asset::Flag(1.0f)},
	};

	var fspath = fs::path(path);

	var metaPath = fspath.parent_path().string() + "//" + fspath.stem().string() + ".meta";

	if (fs::exists(metaPath))
	{
		string data = tmfs::loadFileString(metaPath);
		if (!data.empty()) {
			var meta = json::parse(data);

			for (auto basic_jsons : meta)
			{
				var name = basic_jsons["name"];

				flags[name.get<string>()] = basic_jsons;
			}
		}
	}

	if (cachedFiles[type].Contains(path))
	{
		flags = cachedFiles[type][path]->flags;

		switch (cachedFiles[type][path]->type)
		{
		case Image:
		{
			var tex = cachedTextures[path];
			cachedTextures.Remove(path);
		}
		break;
		case Model:
		{
		}
		break;
		}

		delete cachedFiles[type][path];
	}

	var asset = new Asset;

	asset->type = type;

	asset->path = path;
	asset->lastWriteTime = std::filesystem::last_write_time(path);
	asset->fileName = fs::path(path).filename().string();

	switch (asset->type)
	{
	case Image:
	{
		asset->flags["Flip"] = flags["Flip"];
		asset->flags["TextureType"] = flags["TextureType"];

		var settings = tmTexture::tmTextureSettings();
		settings.type = asset->flags["TextureType"].value.int_0;

		var tex = new tmTexture(path, flags["Flip"].value.bool_0, true, settings);
		cachedTextures[path] = tex;
	}
	break;
	case Model:
	{
		asset->flags["Model Scale"] = flags["Model Scale"];
		cachedModels.Add(mkpair(asset->path, tmModel::LoadModel(asset->path)));
	}
	break;
	case Material:
		{



		}
		break;
	case Script:
		{
		ReloadProjectAssembly();
		}
		break;
	}

	var pair = std::pair<string, Asset*>(path, asset);

	//asset->type = type;


	cachedFiles[asset->type][asset->path] = asset;

	if (true)
	{
		var meta = json();

		for (auto flag : flags)
		{
			json j = flag.second;
			j["name"] = flag.first;

			meta.push_back(j);
		}

		tmfs::writeFileString(metaPath, meta.dump(2));
	}
}

void ResourceManager::ReloadAsset(string path)
{
	ImportNewAsset(path);
}

const char* componenttemplate = R"(
using System;
using TomatoScript.Tomato;

namespace TomatoAssembly
{
    public class NewComponent : MonoComponent
    {
        protected override void Start()
        {
            base.Start();
        }
        protected override void Update()
        {

            base.Update();
        }
    }
}

)";

void ResourceManager::CreateNewAsset(string folder, AssetType type)
{
	var dirPath = assetPath + "//" + folder;

	string filename = "";
	string data = "";

	switch (type)
	{
	case Script:
	{
		filename = "NewScript.cs";
		data = componenttemplate;
	}
	break;
	case Material:
		{
		nlohmann::json j;

		filename = "NewMaterial.mat";
		data = j.dump(2);
		}
		break;
	}

	tmfs::writeFileString(dirPath + "//" + filename, data);

}

void ResourceManager::CheckEdits()
{


	int ctr = 0;
	if (cachedFiles.size() > 0)
	{
		for (auto& [cached_type, cached_dict] : cachedFiles)
		{
			for (auto [cachedPath, cached_file] : cached_dict.GetVector())
			{
				if (cached_file)
				{
					if (std::filesystem::exists(cached_file->path))
					{
						std::replace(cached_file->path.begin(), cached_file->path.end(), '/', '\\');
						std::filesystem::file_time_type lwt = std::filesystem::last_write_time(cached_file->path);
						if (cached_file->lastWriteTime != lwt)
						{
							//std::cout << cached_file->path << " was modified, running " << Asset::AssetTypeToString(cached_file->type) << " processor" << std::endl;
							std::cout << "Reloading asset at " << cached_file->path << std::endl;
							ReloadAsset(cachedPath);
						}
					}
					else
					{
						cached_dict.Remove(cachedPath);

						switch (cached_file->type)
						{
						case Image:
						{
							cachedTextures.Remove(assetPath);
						}
						break;
						case Model:
						{
							cachedModels.Remove(assetPath);
						}
						break;
						case ScriptAssembly:
							tmeGetCore()->scriptMgr->assemblies.Remove(cachedPath);
							break;
						}
					}
				}
				ctr++;
			}
		}
	}

	bool changed = false;
	bool changed_scripts = false;
	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(currentDirectory))
	{
		std::filesystem::path dirPath(dirEntry);
		if (!dirEntry.is_directory())
		{
			string path = dirPath.string();
			if (checkext(meta))
			{
				continue;
			}

			var type = GetAssetTypeFromPath(dirPath.string());
			if (!IsFileCached(dirPath.string()))
			{
				changed = true;
				if (type == Script || type == ScriptAssembly)
				{
					changed_scripts = true;
				}
				ImportNewAsset(dirPath.string());
			}
		}
		else
		{
			if (!cachedFolders.Contains(dirPath))
				cachedFolders.Add(dirPath);
		}
	}

	if (changed) {
		var files = std::unordered_map<int, std::vector<string>>();

		for (auto cachedDict : cachedFiles)
		{
			std::vector<string> paths;
			for (auto cachedFile : cachedDict.second.GetVector())
			{
				paths.push_back(cachedFile.second->path);
			}

			files.insert({ (int)cachedDict.first, paths });

		}

		ImGui::InternalTM::SetFiles(files);

		if (changed_scripts)
		{
			ReloadProjectAssembly();
		}
	}
}

void ResourceManager::Update(bool check)
{
	if (check)
		CheckEdits();
}

AssetType ResourceManager::GetAssetTypeFromPath(string path)
{
	if (checkext(png) || checkext(jpeg))
	{
		return Image;
	} else if (checkext(fbx) || checkext(smd))
	{
		return Model;
	}	else if (checkext(smd))
	{
		return Animation;
	} else if (checkext(cs))
	{
		return Script;
	} else if (checkext(dll))
	{
		return ScriptAssembly;
	}

	return File;
}

MaterialEditor::InoutAttribute::InoutAttribute(InOutType t, string n): type(t), name(n)
{
	data.int_0 = 0;
}

MaterialEditor::MaterialEditor()
{
	ImNodes::CreateContext();
}

teEditorMgr::teProject::teProject(string name)
{
	currentProject = this;
	this->name = name;

	string projectHome = "C:/Users/chris/" "/tomato_projects/";

	var projectDirectory = projectHome + name + "/";

	fs::create_directory(projectHome);
	fs::create_directory(projectDirectory);

	fs::create_directory(projectDirectory + "/Assets/");
	fs::create_directory(projectDirectory + "/Lib/");
	fs::create_directory(projectDirectory + "/Lib/Assemblies/");
	fs::create_directory(projectDirectory + "/Lib/Project/");

	assetPath = projectDirectory + "/Assets/";
	libraryPath = projectDirectory + "/Lib/";

	tmfs::copyFile("scriptcore/TomatoScript.dll", projectDirectory + "Lib/Assemblies/TomatoScript.dll");

	{
		
	}

	tmfs::copyDirectory("resources/icons/", libraryPath + "Icons/");

	tmeGetCore()->scriptMgr->mainAssembly = tmeGetCore()->scriptMgr->LoadAssembly(
		projectDirectory + "Lib/Assemblies/TomatoScript.dll");
	Logger::logger << "Loaded main assembly." << std::endl;

	resMgr = new ResourceManager(assetPath);
}

void teEditorMgr::teProject::Save()
{
	
}


void ResourceManager::ReloadProjectAssembly()
{

	var projPath = teEditorMgr::currentProject->libraryPath;
	var libPath = fs::path(projPath + "/Project/");
	string msBuildPath = MSBUILD_HOME;

	RunCommand(("cd " + libPath.string() + " && premake5 vs2022").c_str());
	RunCommand(("cd " + msBuildPath + " && MsBuild \"" + std::filesystem::absolute(libPath.string()+"/TomatoAssembly.sln").string() + "\" /p:Platform=\"x64\" /p:Configuration=Release").c_str());

	var scriptMgr = tmeGetCore()->scriptMgr;

	var assemblyPath = libPath.string() + "Bin/Release/TomatoAssembly.dll";

	if (!scriptMgr->assemblies.Contains(assemblyPath))
	{
		scriptMgr->LoadAssembly(assemblyPath);
	}

	scriptMgr->Reload();
}

void ResourceManager::RebuildScriptProject()
{
	var projectDirectory = teEditorMgr::currentProject->libraryPath;
	string projectName = "TomatoAssembly";

	//string mainTemplate = tmfs::loadFileString("projectsys/proj_main.cs");

	string premakeTemplate = tmfs::loadFileString("projectsys/generate_proj.lua");

	premakeTemplate = ReplaceAll(premakeTemplate, "_PROJECTNAME", projectName);
	premakeTemplate = ReplaceAll(premakeTemplate, "ConsoleApp", "SharedLib");
	premakeTemplate = ReplaceAll(premakeTemplate, "generated/", "./");
	premakeTemplate = ReplaceAll(premakeTemplate, "**.cs", "../../Assets/**.cs");
	//mainTemplate = ReplaceAll(mainTemplate, "_PROJECTNAME", projectName);

	tmfs::writeFileString(projectDirectory + "/Project/premake5.lua", premakeTemplate);
	tmfs::copyFile("premake5.exe", projectDirectory + "/Project/premake5.exe");
	tmfs::copyFile("scriptcore/TomatoScript.dll", projectDirectory + "/Project/TomatoScript.dll");
}


void teEditorMgr::teCamera::Update(glm::vec2 min, glm::vec2 max, bool block)
{
	const float cameraSpeed = moveSpeed * tmEngine::tmTime::deltaTime;
	if (TM_KEY_PRESSED(TM_KEY_W) || TM_KEY_PRESSED(TM_KEY_UP))
		position += cameraSpeed * front;
	if (TM_KEY_PRESSED(TM_KEY_S) || TM_KEY_PRESSED(TM_KEY_DOWN))
		position -= cameraSpeed * front;
	if (TM_KEY_PRESSED(TM_KEY_A) || TM_KEY_PRESSED(TM_KEY_LEFT))
		position -= glm::normalize(glm::cross(front, up)) * cameraSpeed;
	if (TM_KEY_PRESSED(TM_KEY_D) || TM_KEY_PRESSED(TM_KEY_RIGHT))
		position += glm::normalize(glm::cross(front, up)) * cameraSpeed;

	bool check = tmInput::getMouseButton(TM_MOUSE_BUTTON_RIGHT) == TM_PRESS;

	glm::vec2 delta = {mousePosition.x - lastMousePosition.x, lastMousePosition.y - mousePosition.y};

	lastMousePosition = mousePosition;
	mousePosition = glm::clamp(tmInput::getMousePosition(), min, max);

	if (check && !block)
	{
		float sensitivity = 0.25f;
		glm::vec2 deltaSensitivity = delta * sensitivity;

		rotation.y += deltaSensitivity.x;
		rotation.x += deltaSensitivity.y;

		float pitch = rotation.x;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		rotation.x = pitch;
	}

	glm::vec3 direction;
	direction.x = cos(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
	direction.y = sin(glm::radians(rotation.x));
	direction.z = sin(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
	front = glm::normalize(direction);

	view = glm::lookAt(position, position + front, up);

	framebuffer->cameraPosition = position;

	var engine = tmeGetCore();

	float aspect = framebuffer->size.x / framebuffer->size.y;

	proj = glm::perspective(glm::radians(FieldOfView), aspect, 0.01f, FarPlane);
	orthoProj = glm::ortho(0.0f, framebuffer->size.x, 0.f, framebuffer->size.y, -1.f, 1.f);

	switch (Projection)
	{
	case Perspective:
		break;
	case Orthographic:
		//proj = glm::ortho
		break;
	}

	Use();

	engine->lighting->skybox->draw(this);

	for (auto material : tmeGetCore()->renderMgr->materials)
	{
		UpdateShader(material->shader, material->settings.useOrtho);
	}

	var renderMgr = tmeGetCore()->renderMgr;
	renderMgr->Draw(this);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void teEditorMgr::DrawUI()
{
	// Draw dockspace

	tmCamera::GetMainCamera()->doRender = false;

	if (currentProject)
	{
		currentProject->resMgr->Update(do_reloading);
	}

	_drawDockspace();

	_drawMenuBar();

	if (currentProject)
	{
		_drawSceneView();
		_drawDebug();
		_drawLog();
		_drawExplorer();
		_drawProperties();
		_drawAssetMenu();
		//_drawMaterialEditor();
	}
	else
	{
		if (true)
		{
			ImGui::OpenPopup("FolderPopup");
			projectPopupOpen = true;
		}

		ImVec2 wsiz = {tmeGetCore()->ScreenWidth, tmeGetCore()->ScreenHeight};

		static float div = 2.5;

		ImVec2 winSize = {tmeGetCore()->ScreenWidth / div, tmeGetCore()->ScreenHeight / (div * 1.5f)};

		float aspect = winSize.x / winSize.y;

		ImGui::SetNextWindowSize(winSize);
		//ImGui::SetNextWindowPos({ wsiz.x * 0.5f, wsiz.y * 0.5f });

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, 0, ImVec2(0.5f, 0.5f));

		static var ico = new tmTexture("resources/icons/tom.png", true);

		if (ImGui::BeginPopupModal("FolderPopup", 0,
		                           ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse |
		                           ImGuiWindowFlags_NoDecoration))
		{
			div = std::clamp(div, 1.5f, 5.0f);

			ImGui::Image(ImTextureID(ico->id), ImVec2(75, 75) * aspect, ImVec2(0, 1), ImVec2(1, 0));

			static bool new_project_open = false;

			ImVec2 buttonSize = ImVec2(winSize.x - (75 * aspect) - (15 * aspect), 32.5 * aspect);

			ImGui::SameLine();

			static char buffer[256] = "New Project";

			if (!new_project_open)
			{
				if (ImGui::Button("New", buttonSize))
				{
					new_project_open = true;
					//buffer = (char*)malloc(256 * sizeof(char));
				}
			}
			else
			{
				ImGui::SetNextItemWidth(buttonSize.x - ImGui::CalcTextSize("Project Name").x);

				if (ImGui::InputText("Project Name", buffer, 256,
				                     ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_EnterReturnsTrue))
				{
					Logger::logger << "project " << buffer << std::endl;
					new_project_open = false;

					currentProject = new teEditorMgr::teProject(buffer);
				}
			}

			ImGui::EndPopup();
		}
	}

	ImGui::End();

	var engine = tmeGetCore();

	engine->renderMgr->Clear();
}
