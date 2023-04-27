#pragma once

#include "css.h"

#include "../Module/Modules/Module.h"

#include "../Draw/imgui.h"
#include "../Draw/imgui_impl_dx11.h"
#include "../Draw/imgui_impl_win32.h"

class ClickGui {
private:
	int totalMenuCnt = 7;
	float padding = 90.f;
	float height = 38.f;
public:
	float GetPadding() { return this->padding; }
	float GetHeight() { return this->height; }
	float GetTotalMenuCount() { return this->totalMenuCnt; }
	css GetCSS() { return ceses; }
	ImVec2 GetMenuSize();
	const char* CatToName(Category c);
	int CatToOrd(Category c);
public:
	void RenderMenu();
	void RenderToggle(const char* name, bool* toggle);
	void RenderSettings();
	void RenderModSettings(SModule* mod);
	void RenderToolTip(const char* ttp);
	void RenderCategory(Category c);
public:
	void SetPadding(float to) { this->padding = to; }
	void SetHeight(float to) { this->height = to; }
};

inline ClickGui clg = ClickGui();