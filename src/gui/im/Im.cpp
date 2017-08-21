/*
 * Im.cpp
 *
 *  Created on: May 21, 2017
 *      Author: nullifiedcat
 */

#include "Im.hpp"
#include "Schema.hpp"
#include "Playerlist.hpp"
#include "InfoPanel.hpp"

#include "../../common.h"

#include "imgui.h"

namespace menu { namespace im {

void CatVar_Integer(CatVar& var) {
	const char* label = var.name.c_str();
	if (var.desc_short.size() > 0) label = var.desc_short.c_str();

	float minval = 0;
	float maxval = 100;
	if (var.restricted) {
		minval = var.min;
		maxval = var.max;
	}

	ImGui::PushID(var.id);

	int value = var;
	ImGui::PushItemWidth(120.0f);
	int step = var.restricted ? (var.max - var.min) / 50 : 1;
	if (step == 0) step = 1;
	if (ImGui::InputInt(label, &value, step, step * 20)) {
		var = value;
	}
	/*if (ImGui::SliderInt(label, &value, minval, maxval)) {
		var = value;
	}*/
	if (ImGui::IsItemHovered() && var.desc_long.size()) {
		ImGui::BeginTooltip();
			ImGui::Text("%s", var.desc_long.c_str());
		ImGui::EndTooltip();
	}

	ImGui::PopID();
}

void CatVar_Float(CatVar& var) {
	const char* label = var.name.c_str();
	if (var.desc_short.size() > 0) label = var.desc_short.c_str();

	float minval = 0;
	float maxval = 100;
	if (var.restricted) {
		minval = var.min;
		maxval = var.max;
	}

	ImGui::PushID(var.id);

	float value = var;
	ImGui::PushItemWidth(120.0f);
	float step = var.restricted ? (var.max - var.min) / 50 : 1;
	if (ImGui::InputFloat(label, &value, step, step * 20)) {
		var = value;
	}
	/*if (ImGui::SliderFloat(label, &value, minval, maxval)) {
		var = value;
	}*/
	if (ImGui::IsItemHovered() && var.desc_long.size()) {
		ImGui::BeginTooltip();
			ImGui::Text("%s", var.desc_long.c_str());
		ImGui::EndTooltip();
	}

	ImGui::PopID();
}

void CatVar_Enum(CatVar& var) {
	const char* label = var.name.c_str();
	if (var.desc_short.size() > 0) label = var.desc_short.c_str();

	ImGui::PushID(var.id);

	int value = var;
	const char** items = new const char*[var.enum_type->value_names.size()];
	for (int i = 0; i < var.enum_type->value_names.size(); i++) {
		items[i] = var.enum_type->value_names[i].c_str();
	}
	ImGui::PushItemWidth(120.0f);
	if (ImGui::Combo(label, &value, items, var.enum_type->value_names.size())) {
		var = value;
	}
	if (ImGui::IsItemHovered() && var.desc_long.size()) {
		ImGui::BeginTooltip();
			ImGui::Text(var.desc_long.c_str());
		ImGui::EndTooltip();
	}

	ImGui::PopID();
}

void CatVar_String(CatVar& var) {
	const char* label = var.name.c_str();
	if (var.desc_short.size() > 0) label = var.desc_short.c_str();

	ImGui::PushID(var.id);

	char buf[256] { 0 };
	snprintf(buf, 255, "%s", var.GetString());
	ImGui::PushItemWidth(100.0f);
	if (ImGui::InputText(label, buf, 256)) {
		var.SetValue(std::string(buf));
	}

	if (ImGui::IsItemHovered() && var.desc_long.size()) {
		ImGui::BeginTooltip();
			ImGui::Text("%s", var.desc_long.c_str());
		ImGui::EndTooltip();
	}

	ImGui::PopID();
}

void CatVar_Key(CatVar& var) {
	static const char** keys_array = nullptr;
	if (!keys_array) {
		keys_array = new const char*[ButtonCode_t::BUTTON_CODE_COUNT];
		for (int i = 0; i < ButtonCode_t::BUTTON_CODE_COUNT; i++) {
			keys_array[i] = g_IInputSystem->ButtonCodeToString(ButtonCode_t(i));
		}
	}

	const char* label = var.name.c_str();
	if (var.desc_short.size() > 0) label = var.desc_short.c_str();

	ImGui::PushID(var.id);

	int value = var;
	ImGui::PushItemWidth(75.0f);
	if (ImGui::Combo(label, &value, keys_array, BUTTON_CODE_COUNT)) {
		var = value;
	}
	if (ImGui::IsItemHovered() && var.desc_long.size()) {
		ImGui::BeginTooltip();
			ImGui::Text("%s", var.desc_long.c_str());
		ImGui::EndTooltip();
	}

	ImGui::PopID();

}

void CatVar_Switch(CatVar& var) {
	const char* label = var.name.c_str();
	if (var.desc_short.size() > 0) label = var.desc_short.c_str();

	ImGui::PushID(var.id);

	bool value = var;
	if (ImGui::Checkbox(label, &value)) {
		var = value;
	}
	if (ImGui::IsItemHovered() && var.desc_long.size()) {
		ImGui::BeginTooltip();
			ImGui::Text("%s", var.desc_long.c_str());
		ImGui::EndTooltip();
	}

	ImGui::PopID();
}

void RenderList(ListEntry_List* list) {
	int column = 0;
	for (const auto& j : list->entries) {
		switch (j->type) {
		case VARIABLE: {
			if (!column) {
				ImGui::Columns(3, nullptr, 0);
			}
			CatVar& v = ((ListEntry_Variable*)j.get())->var;
			switch (v.type) {
			case CV_SWITCH: CatVar_Switch(v); break;
			case CV_INT: CatVar_Integer(v); break;
			case CV_FLOAT: CatVar_Float(v); break;
			case CV_STRING: CatVar_String(v); break;
			case CV_KEY: CatVar_Key(v); break;
			case CV_ENUM: CatVar_Enum(v); break;
			}
			if (column < 2) {
				ImGui::NextColumn();
				column++;
			} else {
				column = 0;
			}
		} break;
		case LABEL:
			ImGui::Text(((ListEntry_Label*)j.get())->text.c_str());
			break;
		case SUBLIST:
			ImGui::Columns(1);
			column = 0;
			if (ImGui::CollapsingHeader(((ListEntry_List*)j.get())->name.c_str())) {
				RenderList((ListEntry_List*)j.get());
			}
		}
	}
}

int active_list = 0;
	//(color_for_text, color_for_head, color_for_area, color_for_body, color_for_pops)
// Settings use Easy theming
CatVar imgui_custom(CV_SWITCH, "imgui_custom", "0", "Custom ImGui", "Allows usersettings to overwrite the default imgui style");
	
CatEnum imgui_color_text_enum({ "White", "Black" });
CatVar imgui_color_text(imgui_color_text_enum, "imgui_color_text", "0", "Text", "Text Color");
	
CatVar imgui_color_head_r(CV_INT, "imgui_color_head_r", "13", "Head Color: (red)", "Defines red component of imgui head color", 100);
CatVar imgui_color_head_g(CV_INT, "imgui_color_head_g", "10", "Head Color: (green)", "Defines green component of imgui head color", 100);
CatVar imgui_color_head_b(CV_INT, "imgui_color_head_b", "15", "Head Color: (blue)", "Defines blue component of imgui head color", 100);
	
CatVar imgui_color_area_r(CV_INT, "imgui_color_area_r", "10", "Area Color: (red)", "Defines red component of imgui area color", 100);
CatVar imgui_color_area_g(CV_INT, "imgui_color_area_g", "9", "Area Color: (green)", "Defines green component of imgui area color", 100);
CatVar imgui_color_area_b(CV_INT, "imgui_color_area_b", "12", "Area Color: (blue)", "Defines blue component of imgui area color", 100);
	
CatVar imgui_color_body_r(CV_INT, "imgui_color_body_r", "6", "Body Color: (red)", "Defines red component of imgui body color", 100);
CatVar imgui_color_body_g(CV_INT, "imgui_color_body_g", "5", "Body Color: (green)", "Defines green component of imgui body color", 100);
CatVar imgui_color_body_b(CV_INT, "imgui_color_body_b", "7", "Body Color: (blue)", "Defines blue component of imgui body color", 100);
	
CatVar imgui_color_pops_r(CV_INT, "imgui_color_pops_r", "7", "Pops Color: (red)", "Defines red component of imgui pops color", 100);
CatVar imgui_color_pops_g(CV_INT, "imgui_color_pops_g", "7", "Pops Color: (green)", "Defines green component of imgui pops color", 100);
CatVar imgui_color_pops_b(CV_INT, "imgui_color_pops_b", "9", "Pops Color: (blue)", "Defines blue component of imgui pops color", 100);	
	

struct ImVec3 { float x, y, z; ImVec3(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) { x = _x; y = _y; z = _z; } };

void imgui_easy_theming(ImVec3 color_for_text, ImVec3 color_for_head, ImVec3 color_for_area, ImVec3 color_for_body, ImVec3 color_for_pops)
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.Colors[ImGuiCol_Text] = ImVec4( color_for_text.x, color_for_text.y, color_for_text.z, 1.00f );
    style.Colors[ImGuiCol_TextDisabled] = ImVec4( color_for_text.x, color_for_text.y, color_for_text.z, 0.58f );
    style.Colors[ImGuiCol_WindowBg] = ImVec4( color_for_body.x, color_for_body.y, color_for_body.z, 1.00f );
    style.Colors[ImGuiCol_ChildWindowBg] = ImVec4( color_for_area.x, color_for_area.y, color_for_area.z, 0.58f );
    style.Colors[ImGuiCol_Border] = ImVec4( color_for_body.x, color_for_body.y, color_for_body.z, 0.00f );
    style.Colors[ImGuiCol_BorderShadow] = ImVec4( color_for_body.x, color_for_body.y, color_for_body.z, 0.00f );
    style.Colors[ImGuiCol_FrameBg] = ImVec4( color_for_area.x, color_for_area.y, color_for_area.z, 1.00f );
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.78f );
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 1.00f );
    style.Colors[ImGuiCol_TitleBg] = ImVec4( color_for_area.x, color_for_area.y, color_for_area.z, 1.00f );
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4( color_for_area.x, color_for_area.y, color_for_area.z, 0.75f );
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 1.00f );
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4( color_for_area.x, color_for_area.y, color_for_area.z, 0.47f );
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4( color_for_area.x, color_for_area.y, color_for_area.z, 1.00f );
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.21f );
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.78f );
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 1.00f );
    style.Colors[ImGuiCol_ComboBg] = ImVec4( color_for_area.x, color_for_area.y, color_for_area.z, 1.00f );
    style.Colors[ImGuiCol_CheckMark] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.80f );
    style.Colors[ImGuiCol_SliderGrab] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.50f );
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 1.00f );
    style.Colors[ImGuiCol_Button] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.50f );
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.86f );
    style.Colors[ImGuiCol_ButtonActive] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 1.00f );
    style.Colors[ImGuiCol_Header] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.76f );
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.86f );
    style.Colors[ImGuiCol_HeaderActive] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 1.00f );
    style.Colors[ImGuiCol_Column] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.32f );
    style.Colors[ImGuiCol_ColumnHovered] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.78f );
    style.Colors[ImGuiCol_ColumnActive] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 1.00f );
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.15f );
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.78f );
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 1.00f );
    style.Colors[ImGuiCol_CloseButton] = ImVec4( color_for_text.x, color_for_text.y, color_for_text.z, 0.16f );
    style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4( color_for_text.x, color_for_text.y, color_for_text.z, 0.39f );
    style.Colors[ImGuiCol_CloseButtonActive] = ImVec4( color_for_text.x, color_for_text.y, color_for_text.z, 1.00f );
    style.Colors[ImGuiCol_PlotLines] = ImVec4( color_for_text.x, color_for_text.y, color_for_text.z, 0.63f );
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 1.00f );
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4( color_for_text.x, color_for_text.y, color_for_text.z, 0.63f );
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 1.00f );
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.43f );
    style.Colors[ImGuiCol_PopupBg] = ImVec4( color_for_pops.x, color_for_pops.y, color_for_pops.z, 1.00f );
    style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4( color_for_area.x, color_for_area.y, color_for_area.z, 0.73f );
}

#define IM3_RGB(r,g,b) ImVec3((float)r / 255.f, (float)g / 255.f, (float)b / 255.f)

void Render() {
	static bool styles_setup = false;
	if (!styles_setup) {
		ImGuiStyle * style = &ImGui::GetStyle();

		style->WindowPadding = ImVec2(15, 15);
		//style->WindowRounding = 5.0f;
		style->WindowRounding = 1.0f;

		style->FramePadding = ImVec2(5, 5);
		//style->FrameRounding = 4.0f;
		style->FrameRounding = 1.0f;

		style->ItemSpacing = ImVec2(12, 8);
		//style->ItemInnerSpacing = ImVec2(8, 6);
		style->ItemInnerSpacing = ImVec2(6, 6);
		style->IndentSpacing = 25.0f;
		style->ScrollbarSize = 15.0f;
		//style->ScrollbarRounding = 9.0f;
		style->ScrollbarRounding = 1.0f;

		style->GrabMinSize = 5.0f;
		//style->GrabRounding = 3.0f;
		style->GrabRounding = 1.0f;


		style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
		style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
		style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
		style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
		style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_ComboBg] = ImVec4(0.19f, 0.18f, 0.21f, 1.00f);
		style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_CloseButton] = ImVec4(0.40f, 0.39f, 0.38f, 0.16f);
		style->Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.40f, 0.39f, 0.38f, 0.39f);
		style->Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
		style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
		style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
		styles_setup = true;
	}
	
	// Info panel should be rendered even with main gui disabled
	//RenderInfoPanel();
	
	if (!gui_visible) {
		ImGui::GetIO().MouseDrawCursor = false;
		return;
	} else {
		ImGui::GetIO().MouseDrawCursor = true;
		ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
	}
	
	// Check if usersettings allow custom ui
	if (imgui_custom) {
		
		// Create ImVectors from user settings 
		// "White", "Black"  true false
		ImVec3 imgui_text(0,0,0);
		if (!imgui_color_text)
			imgui_text = ImVec3(0.80, 0.80, 0.83); // White
		else
			imgui_text = ImVec3(0.05, 0.05, 0.07); // Black
		// Multiply by .01 to get im colors from whole numbers
		ImVec3 imgui_head((int)imgui_color_head_r * 0.01, (int)imgui_color_head_g * 0.01, (int)imgui_color_head_b * 0.01);
		ImVec3 imgui_area((int)imgui_color_area_r * 0.01, (int)imgui_color_area_g * 0.01, (int)imgui_color_area_b * 0.01);
		ImVec3 imgui_body((int)imgui_color_body_r * 0.01, (int)imgui_color_body_g * 0.01, (int)imgui_color_body_b * 0.01);
		ImVec3 imgui_pops((int)imgui_color_pops_r * 0.01, (int)imgui_color_pops_g * 0.01, (int)imgui_color_pops_b * 0.01);
							  
		// Push users theme settings
		imgui_easy_theming(imgui_text, imgui_head, imgui_area, imgui_body, imgui_pops);
		
		// Make the gui reset to default next time around
		styles_setup = false;
	}
	
	
	if (gui_debug)
		ImGui::ShowTestWindow();
	RenderPlayerlist();
	/*ImGui::Begin("Colors");
		static ImVec3 color_for_text = IM3_RGB(246, 236, 255);
		static ImVec3 color_for_head = IM3_RGB(209, 96, 165);
		static ImVec3 color_for_area = IM3_RGB(104, 57, 78);
		static ImVec3 color_for_body = IM3_RGB(80, 44, 68);
		static ImVec3 color_for_pops = IM3_RGB(58, 33, 59);
		if (ImGui::ColorEdit3("Text", &color_for_text.x) ||
			ImGui::ColorEdit3("Head", &color_for_head.x) ||
			ImGui::ColorEdit3("Area", &color_for_area.x) ||
			ImGui::ColorEdit3("Body", &color_for_body.x) ||
			ImGui::ColorEdit3("Pops", &color_for_pops.x)) {
		    imgui_easy_theming(color_for_text, color_for_head, color_for_area, color_for_body, color_for_pops);
		}
	ImGui::End();*/
	if (!main_list_array.size()) return;
	ImGui::Begin("Config Menu");
	ImGui::SetWindowSize(ImVec2{900, 600});
	if (active_list >= main_list_array.size()) active_list = 0;
	for (int i = 0; i < main_list_array.size(); i++) {
		if (ImGui::Button(main_list_array.at(i)->name.c_str())) {
			active_list = i;
		}
		ImGui::SameLine();
	}
	ImGui::NewLine();
	RenderList(main_list_array.at(active_list));
	ImGui::End();
}

}}
