#pragma once

#include "css.h"

#include "../Module/Modules/Module.h"

#include "../Draw/imgui.h"
#include "../Draw/imgui_impl_dx11.h"
#include "../Draw/imgui_impl_win32.h"

class ClickGui {
private:
	int totalMenuCnt = 9;
	float padding = 30.f;
	float height = 30.f;
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
	void ToggleButton(const char* name, bool* enb);
	void RenderSettings();
	void RenderCategory(Category c);
	void RenderTooltip(const char* ttp);
public:
	void SetPadding(float to) { this->padding = to; }
	void SetHeight(float to) { this->height = to; }
};

inline ClickGui clg = ClickGui();