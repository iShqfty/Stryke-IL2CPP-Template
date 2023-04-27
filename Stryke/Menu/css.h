#pragma once

#include "../Draw/imgui.h"
#include "../Draw/imgui_impl_dx11.h"
#include "../Draw/imgui_impl_win32.h"

class css {
private:
	ImVec4 themeCol = ImVec4(0.05098039284348488f, 0.1137254908680916f, 0.1882352977991104f, 1.0f);
	ImVec4 themeHov = ImVec4(0.196078434586525f, 0.3921568691730499f, 0.5882353186607361f, 1.0f);
	ImVec4 themeAct = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);

	ImVec4 textCol = ImVec4(0.7058823529411765, 0.7058823529411765, 0.7058823529411765, 1.f);
	ImVec4 textAct = ImVec4(1.f, 1.f, 1.f, 1.f);
public:
	ImVec4 GetThemeColor() { return this->themeCol; }
	ImVec4 GetThemeHovered() { return this->themeHov; }
	ImVec4 GetThemeActive() { return this->themeAct; }

	ImVec4 GetTextColor() { return this->textCol; }
	ImVec4 GetTextActive() { return this->textAct; }
private:
	float opacity = 1.f;
public:
	float GetOpactiy() { return opacity; }
public:
	void SetOpacity(float to) { this->opacity = to; }
};

inline css ceses = css();