#include "imgui_ext.hpp"

#include "engine.hpp"
#include "imgui.h"
#include "tmgl.h"

#include "util/input.h"
#include "imgui/misc/cpp/imgui_stdlib.h"

#define imtogm(v) glm::vec2(v.x, v.y)
#define gmtoim(v) ImVec2(v.x, v.y)

std::unordered_map<int, std::vector<string>>global_files;
std::string filter = "";

string ImGui::FilePicker(int type, string pa, bool& found)
{
	string p = pa;

	std::vector<string> paths;

	for (auto global_file : global_files)
	{
		if (global_file.first == type)
		{
			paths = global_file.second;
			break;
		}
	}

	vec2 mousePos = tmInput::getMousePosition() - tmGetCore()->getWindowPos();

	ImVec2 center = gmtoim( mousePos );
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.f, 0.f));

	float w = 0.25;
	float h = 0.35;

	ImVec2 size = ImVec2(tmeGetCore()->ScreenWidth * w, tmeGetCore()->ScreenHeight * h);

	float aspect = ImGui::GetWindowWidth() / ImGui::GetWindowHeight();
	var iconSize = ImVec2(250, 250) * aspect;

	ImGui::SetNextWindowSize(size);

	if (ImGui::BeginPopupModal("ChooseAsset", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove))
	{

		ImGui::InputText(ICON_FA_MAGNIFYING_GLASS "Search", &filter);

		ImGui::Columns(3,0,false);

		if (ImGui::IsKeyDown(ImGuiKey_Escape))
		{
			ImGui::CloseCurrentPopup();
		}

		{
			string name = "<NONE>";
				
			bool clicked = ImGui::ImageButton(name.c_str(), (ImTextureID)0, iconSize - (ImGui::GetStyle().FramePadding * 2.f));
			ImGui::SetNextItemWidth(iconSize.x - (ImGui::GetStyle().WindowPadding.x * 2.f));
			ImGui::Text(name.c_str());

			if (clicked)
			{
				p = "";
				found = true;

				ImGui::CloseCurrentPopup();
			}
		}

		for (auto path : paths)
		{
			var name = fs::path(path).filename().string();

			if (!StringContains(name, filter))
				continue;

			ImTextureID id = 0;

			var tex = tmTexture::CheckTexture(path);
			if (tex)
				id = (ImTextureID)tex->id;

			bool clicked = ImGui::ImageButton(name.c_str(),id, iconSize - (ImGui::GetStyle().FramePadding * 2.f));
			ImGui::SetNextItemWidth(iconSize.x - (ImGui::GetStyle().WindowPadding.x * 2.f));
			ImGui::Text(name.c_str());

			if (clicked)
			{
				found = true;
				p = path;

				ImGui::CloseCurrentPopup();

				break;
			}

			ImGui::NextColumn();
		}


		ImGui::EndPopup();
	}

	return p;
}

tmTexture* ImGui::TexturePicker(string name, tmTexture* tex)
{
	static string openName ="";
	float aspect = ImGui::GetWindowWidth() / ImGui::GetWindowHeight();
	var iconSize = ImVec2(250, 250) * aspect;

	bool clicked = ImGui::ImageButton(name.c_str(), (ImTextureID)tex->id, iconSize - (ImGui::GetStyle().FramePadding * 2.f));

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_IMP"))
		{
			string payload_n = *(const string*)payload->Data;



			return tmTexture::CheckTexture(payload_n);

		}
		ImGui::EndDragDropTarget();
	}

	ImGui::SameLine();
	ImGui::SetNextItemWidth(iconSize.x - (ImGui::GetStyle().WindowPadding.x * 2.f));
	ImGui::Text(name.c_str());
	var found = false;
	string p = "";

	if (clicked)
	{
		ImGui::OpenPopup("ChooseAsset");
		openName = name;
		//fieldN = field.name;
	}

	if (openName == name) {
		p = ImGui::FilePicker(1, tex->path, found);
	}


	if (found && openName == name)
	{
		openName = "";
		if (p.empty())
			return GetTexture(0);
		else
			return tmTexture::CheckTexture(p);
	} else if (found)
	{
		std::cout << "Found tex but not matching picker " << openName << ":" << name << std::endl;
	}

	return tex;
}

void ImGui::InternalTM::SetFiles(std::unordered_map<int, std::vector<string>> files)
{
	global_files = files;
}
