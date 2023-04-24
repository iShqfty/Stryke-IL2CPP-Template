#pragma once

#include "ClickGui.h"

#include "../Module/ModManager.h"

#include "../Draw/imgui.h"
#include "../Draw/imgui_impl_dx11.h"
#include "../Draw/imgui_impl_win32.h"

// ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** **

// ** ** ** ** GETTING VARIABLES CATEGORY ** ** ** ** **

// ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** **

const char* ClickGui::CatToName(Category c) {
	const char* ret = "";

	switch (c) {
	case Category::LOBBY:
		ret = "Lobby";
		break;
	case Category::MAP:
		ret = "Map";
		break;
	case Category::PLAYER:
		ret = "Player";
		break;
	case Category::VISUAL:
		ret = "Visual";
		break;
	case Category::MISC:
		ret = "Miscellaneous";
		break;
	}

	return ret;
}

int ClickGui::CatToOrd(Category c) {
	int ret = 0;

	switch (c) {
	case Category::LOBBY:
		ret = 0;
		break;
	case Category::PLAYER:
		ret = 1;
		break;
	case Category::MAP:
		ret = 2;
		break;
	case Category::VISUAL:
		ret = 3;
		break;
	case Category::MISC:
		ret = 4;
		break;
	}

	return ret;
}

ImVec2 ClickGui::GetMenuSize() {
	float totalW = ImGui::GetIO().DisplaySize.x;
	return ImVec2(((totalW - (this->padding * this->totalMenuCnt)) / this->totalMenuCnt), this->height);
}

// ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** **

// ** ** ** ** RENDERING VOID CATEGORY ** ** ** ** ** **

// ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** **

bool ClickGui::ToggleButton(const char* name, bool* enb) {
	ret = false;
	ImVec2 p = ImGui::GetCursorScreenPos();
	ImDrawList* drawlst = ImGui::GetWindowDrawList();
	

	ImVec2 hw = ImVec2(this->GetMenuSize().x, this->height);

	std::string s(name);
	std::string id = "##" + s;

	if (ImGui::InvisibleButton(id.c_str(), hw))
		*enb = !*enb;
		ret = true;
	ImU32 col_bg;
	ImU32 col_txt;
	if (ImGui::IsItemHovered()) {
		col_bg = IM_COL32(this->GetCSS().GetThemeActive().x * 255.f, this->GetCSS().GetThemeActive().y * 255.f, this->GetCSS().GetThemeActive().z * 255.f, this->GetCSS().GetThemeActive().w * 255.f);
		col_txt = *enb ? IM_COL32(this->GetCSS().GetTextColor().x * 255.f, this->GetCSS().GetTextColor().y * 255.f, this->GetCSS().GetTextColor().z * 255.f, this->GetCSS().GetTextColor().w * 255.f) : IM_COL32(this->GetCSS().GetTextActive().x * 255.f, this->GetCSS().GetTextActive().y * 255.f, this->GetCSS().GetTextActive().z * 255.f, this->GetCSS().GetTextActive().w * 255.f);
	}
	else {
		col_bg = *enb ? IM_COL32(this->GetCSS().GetThemeActive().x * 255.f, this->GetCSS().GetThemeActive().y * 255.f, this->GetCSS().GetThemeActive().z * 255.f, this->GetCSS().GetThemeActive().w * 255.f) : IM_COL32(this->GetCSS().GetThemeColor().x * 255.f, this->GetCSS().GetThemeColor().y * 255.f, this->GetCSS().GetThemeColor().z * 255.f, this->GetCSS().GetThemeColor().w * 255.f);
		col_txt = *enb ? IM_COL32(this->GetCSS().GetTextActive().x * 255.f, this->GetCSS().GetTextActive().y * 255.f, this->GetCSS().GetTextActive().z * 255.f, this->GetCSS().GetTextActive().w * 255.f) : IM_COL32(this->GetCSS().GetTextColor().x * 255.f, this->GetCSS().GetTextColor().y * 255.f, this->GetCSS().GetTextColor().z * 255.f, this->GetCSS().GetTextColor().w * 255.f);
	}

	drawlst->AddRectFilled(p, ImVec2(p.x + hw.x, p.y + hw.y), col_bg);
	drawlst->AddText(ImVec2(p.x + 4, p.y + 3), col_txt, name);
	
	return ret;
}

void ClickGui::RenderTooltip(const char* ttp) {
	ImGui::Begin("##tooltipmenu", NULL, 
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove);
	ImGui::SetWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 30), ImGuiCond_Once);
	ImGui::Text(ttp);
	ImGui::End();
}

inline int foundMods = 0;
void ClickGui::RenderCategory(Category c) {
	foundMods = 0;
	ImGui::Begin(this->CatToName(c), NULL,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse
	);
	ImGui::SetWindowPos(ImVec2(((this->padding * (this->CatToOrd(c) + 1)) + (this->GetMenuSize().x * this->CatToOrd(c) + 10)), this->padding), ImGuiCond_Once);
	for (auto& mod : mmgr.mods) {
		if (mod.get()->getCateg() == c) {
			foundMods += 1;
			if (ToggleButton(mod.get()->getName(), &mod.get()->enabled))
				this->RenderTooltip(mod.get()->getDesc());
			mod.get()->checkEnabled();
			if (mod.get()->enabled)
				mod.get()->onFrame();
		}
	}
	ImGui::SetWindowSize(ImVec2(this->GetMenuSize().x, (foundMods+1) * this->GetHeight()), ImGuiCond_Once);
	ImGui::End();
}

void ClickGui::RenderMenu() {
	this->RenderCategory(Category::LOBBY);
	this->RenderCategory(Category::MAP);
	this->RenderCategory(Category::PLAYER);
	this->RenderCategory(Category::VISUAL);
	this->RenderCategory(Category::MISC);
}
