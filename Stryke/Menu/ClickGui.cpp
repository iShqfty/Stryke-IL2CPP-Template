#include "ClickGui.h"

#include "../Module/ModManager.h"
#include "../Module/Modules/Module.h"

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

void ClickGui::RenderToolTip(const char* ttp) {
	ImGui::Begin("##rendertooltip");
	ImGui::Text(ttp);
	ImGui::End();
}

static int inner = 9;
void ClickGui::RenderToggle(const char* name, bool* toggle) {
	ImVec2 p = ImGui::GetCursorScreenPos();
	ImDrawList* drawlst = ImGui::GetWindowDrawList();

	std::string add("###");
	std::string mn(name);
	std::string fin = add + mn;

	if (ImGui::InvisibleButton(fin.c_str(), this->GetMenuSize()))
		*toggle = !*toggle;

	if (ImGui::IsItemHovered())
		drawlst->AddRectFilled(ImVec2(p.x + inner + 1, p.y + inner + 1), ImVec2(p.x + this->GetHeight() - inner - 1, p.y + this->GetHeight() - inner - 2), IM_COL32(200, 200, 200, 200));

	if (*toggle) {
		drawlst->AddRect(ImVec2(p.x + inner, p.y + inner), ImVec2(p.x + this->GetHeight() - inner, p.y + this->GetHeight() - inner), IM_COL32(255, 255, 255, 255));
		drawlst->AddRectFilled(ImVec2(p.x + inner + 2, p.y + inner + 2), ImVec2(p.x + this->GetHeight() - inner - 2, p.y + this->GetHeight() - inner - 2), IM_COL32(this->GetCSS().GetThemeActive().x * 255.f, this->GetCSS().GetThemeActive().y * 255.f, this->GetCSS().GetThemeActive().z * 255.f, this->GetCSS().GetThemeActive().w * 255.f));
		drawlst->AddText(ImVec2(p.x + this->GetHeight() + 0.5, p.y + 8), IM_COL32(255, 255, 255, 255), name);
	}
	else {
		drawlst->AddRect(ImVec2(p.x + inner, p.y + inner), ImVec2(p.x + this->GetHeight() - inner, p.y + this->GetHeight() - inner), IM_COL32(255, 255, 255, 255));
		drawlst->AddText(ImVec2(p.x + this->GetHeight() + 0.5, p.y + 8), IM_COL32(255, 255, 255, 255), name);
	}
}

void ClickGui::RenderModSettings(SModule* mod) {
	for (auto& stg : mod->getSettings()) {
		if (stg->type == SettingType::BOOL_ST)
			this->RenderToggle(stg->desc, &stg->val->_boolst);
		if (stg->type == SettingType::FLOAT_ST)
			ImGui::SliderFloat(stg->desc, &stg->val->_floatst, stg->minVal->_floatst, stg->maxVal->_floatst);
	}
}

static int foundMods = 0;
void ClickGui::RenderCategory(Category c) {
	foundMods = 0;
	ImGui::Begin(this->CatToName(c), NULL,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse
	);
	ImGui::SetWindowPos(ImVec2(((this->padding * (this->CatToOrd(c) + 1)) + (this->GetMenuSize().x * this->CatToOrd(c) + 10)), this->padding), ImGuiCond_Once);
	for (auto& mod : mmgr.mods) {
		if (mod->getCateg() == c) {
			foundMods += 1;
			ImGui::PushStyleColor(ImGuiCol_Header, mod->enabled ? this->GetCSS().GetThemeActive() : this->GetCSS().GetThemeColor());
			if (ImGui::CollapsingHeader(mod->getName())) {
				this->RenderModSettings(mod.get());
				mod->extended = true;
			}
			else
				mod->extended = false;
			ImGui::PopStyleColor();
			mod->checkEnabled();
			if (mod->enabled)
				mod->onFrame();
			if (mod->extended)
				foundMods += mod->getSettings().size();
		}
	}
	ImGui::SetWindowSize(ImVec2(this->GetMenuSize().x, (foundMods+1) * this->GetHeight()));
	ImGui::End();
}

void ClickGui::RenderMenu() {
	this->RenderCategory(Category::LOBBY);
	this->RenderCategory(Category::MAP);
	this->RenderCategory(Category::PLAYER);
	this->RenderCategory(Category::VISUAL);
	this->RenderCategory(Category::MISC);
}
