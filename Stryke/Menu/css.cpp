#pragma once

#include "ClickGui.h"

#include "../Draw/imgui.h"
#include "../Draw/imgui_impl_dx11.h"
#include "../Draw/imgui_impl_win32.h"

void ClickGui::RenderSettings() {

	ImGuiStyle* style = &ImGui::GetStyle();

	style->Alpha = this->GetCSS().GetOpactiy();
	style->WindowPadding = ImVec2(0.0f, 0.0f);
	style->WindowRounding = 0.0f;
	style->WindowBorderSize = 1.0f;
	style->WindowMinSize = ImVec2(32.0f, 32.0f);
	style->WindowTitleAlign = ImVec2(0.0f, 0.5f);
	style->WindowMenuButtonPosition = ImGuiDir_Left;
	style->ChildRounding = 0.0f;
	style->ChildBorderSize = 1.0f;
	style->PopupRounding = 0.0f;
	style->PopupBorderSize = 1.0f;
	style->FramePadding = ImVec2(5.0f, 9.0f);
	style->FrameRounding = 0.0f;
	style->FrameBorderSize = 0.0f;
	style->ItemSpacing = ImVec2(0.0f, 0.0f);
	style->ItemInnerSpacing = ImVec2(0.0f, 0.0f);
	style->IndentSpacing = 15.0f;
	style->ColumnsMinSpacing = 6.0f;
	style->ScrollbarSize = 14.0f;
	style->ScrollbarRounding = 0.0f;
	style->GrabMinSize = 5.0f;
	style->GrabRounding = 20.0f;
	style->TabRounding = 0.0f;
	style->TabBorderSize = 0.0f;
	style->ColorButtonPosition = ImGuiDir_Right;
	style->ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style->SelectableTextAlign = ImVec2(0.0f, 0.0f);


	style->Colors[ImGuiCol_Text] = this->GetCSS().GetTextActive();
	style->Colors[ImGuiCol_TextDisabled] = this->GetCSS().GetTextColor();
	style->Colors[ImGuiCol_WindowBg] = this->GetCSS().GetThemeColor();
	style->Colors[ImGuiCol_ChildBg] = this->GetCSS().GetThemeColor();
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style->Colors[ImGuiCol_FrameBg] = this->GetCSS().GetThemeColor();
	style->Colors[ImGuiCol_FrameBgHovered] = this->GetCSS().GetThemeHovered();
	style->Colors[ImGuiCol_FrameBgActive] = this->GetCSS().GetThemeActive();
	style->Colors[ImGuiCol_TitleBg] = this->GetCSS().GetThemeColor();
	style->Colors[ImGuiCol_TitleBgActive] = this->GetCSS().GetThemeColor();
	style->Colors[ImGuiCol_TitleBgCollapsed] = this->GetCSS().GetThemeColor();
	style->Colors[ImGuiCol_MenuBarBg] = this->GetCSS().GetThemeColor();
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1019607856869698f, 0.2274509817361832f, 0.3764705955982208f, 1.0f);
	style->Colors[ImGuiCol_ScrollbarGrab] = this->GetCSS().GetThemeColor();
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = this->GetCSS().GetThemeHovered();
	style->Colors[ImGuiCol_ScrollbarGrabActive] = this->GetCSS().GetThemeActive();
	style->Colors[ImGuiCol_CheckMark] = ImVec4(0.2599999904632568f, 0.5899999737739563f, 0.9800000190734863f, 1.0f);
	style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.239999994635582f, 0.5199999809265137f, 0.8799999952316284f, 1.0f);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.2599999904632568f, 0.5899999737739563f, 0.9800000190734863f, 1.0f);
	style->Colors[ImGuiCol_Button] = this->GetCSS().GetThemeColor();
	style->Colors[ImGuiCol_ButtonHovered] = this->GetCSS().GetThemeHovered();
	style->Colors[ImGuiCol_ButtonActive] = this->GetCSS().GetThemeActive();
	style->Colors[ImGuiCol_Header] = this->GetCSS().GetThemeColor();
	style->Colors[ImGuiCol_HeaderHovered] = this->GetCSS().GetThemeHovered();
	style->Colors[ImGuiCol_HeaderActive] = this->GetCSS().GetThemeActive();
	style->Colors[ImGuiCol_Separator] = ImVec4(0.4274509847164154f, 0.4274509847164154f, 0.4980392158031464f, 0.5f);
	style->Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.09803921729326248f, 0.4000000059604645f, 0.7490196228027344f, 0.7799999713897705f);
	style->Colors[ImGuiCol_SeparatorActive] = ImVec4(0.09803921729326248f, 0.4000000059604645f, 0.7490196228027344f, 1.0f);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.2000000029802322f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.6700000166893005f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.949999988079071f);
	style->Colors[ImGuiCol_Tab] = ImVec4(0.1764705926179886f, 0.3490196168422699f, 0.5764706134796143f, 0.8619999885559082f);
	style->Colors[ImGuiCol_TabHovered] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.800000011920929f);
	style->Colors[ImGuiCol_TabActive] = ImVec4(0.196078434586525f, 0.407843142747879f, 0.6784313917160034f, 1.0f);
	style->Colors[ImGuiCol_TabUnfocused] = ImVec4(0.06666667014360428f, 0.1019607856869698f, 0.1450980454683304f, 0.9724000096321106f);
	style->Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1333333402872086f, 0.2588235437870026f, 0.4235294163227081f, 1.0f);
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.6078431606292725f, 0.6078431606292725f, 0.6078431606292725f, 1.0f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.4274509847164154f, 0.3490196168422699f, 1.0f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8980392217636108f, 0.6980392336845398f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.6000000238418579f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.3499999940395355f);
	style->Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.8999999761581421f);
	style->Colors[ImGuiCol_NavHighlight] = this->GetCSS().GetThemeActive();
	style->Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
	style->Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
	style->Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.3499999940395355f);
}