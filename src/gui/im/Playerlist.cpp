/*
 * Playerlist.cpp
 *
 *  Created on: May 21, 2017
 *      Author: nullifiedcat
 */

#include "Playerlist.hpp"
#include "../../common.h"
#include "../../playerlist.hpp"

#include "imgui.h"

namespace menu { namespace im {

void RenderPlayer(int eid) {
	ImGui::PushID(eid);
	player_info_s info;
	bool success = g_IEngine->GetPlayerInfo(eid, &info);
	if (success) {
		int x = 0;
		// UserID
		ImGui::Text("%d", info.userID);
		x += 48;
		ImGui::SameLine(x);
		// SteamID
		ImGui::Text("%u", info.friendsID);
		x += 80;
		ImGui::SameLine(x);

		char safename[32];
		for (int i = 0, j = 0; i < 32; i++) {
			if (info.name[i] == 0) {
				safename[j] = 0;
				break;
			}
			if (info.name[i] == '\n') continue;
			safename[j++] = info.name[i];
		}

		ImGui::Text("%s", safename);
		x += 8 * 32;
		ImGui::SameLine(x);

		int iclazz = 0;
		rgba_t bgcolor = colors::empty;
		const char* text = "N/A";
		IF_GAME (IsTF()) {
			iclazz = g_pPlayerResource->GetClass(ENTITY(eid));
			int team = g_pPlayerResource->GetTeam(eid);
			if (eid != g_IEngine->GetLocalPlayer()) {
				if (team == TEAM_RED) {
					bgcolor = colors::red;
				} else if (team == TEAM_BLU) {
					bgcolor = colors::blu;
				}
			}
			if (iclazz && iclazz < 10) {
				text = classes[iclazz - 1].c_str();
			}
		}

		if (bgcolor.a) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(bgcolor.r, bgcolor.g, bgcolor.b, bgcolor.a));
		}

		ImGui::Text("%s", text);
		x += 80;
		ImGui::SameLine(x);

		if (bgcolor.a) {
			ImGui::PopStyleColor();
		}

		playerlist::userdata& data = playerlist::AccessData(info.friendsID);
		int& state = *reinterpret_cast<int*>(&data.state);
		bgcolor = playerlist::Color(info.friendsID);
		if (bgcolor.a) {
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(bgcolor.r, bgcolor.g, bgcolor.b, bgcolor.a));
		}
		ImGui::PushItemWidth(120);
		ImGui::Combo("", &state, playerlist::k_pszNames, 5);
		ImGui::PopItemWidth();

		x += 124;

		if (bgcolor.a) {
			ImGui::PopStyleColor();
		}

		if (backpacktf::enabled()) {
			ImGui::SameLine(x);
			if (info.fakeplayer) {
				ImGui::Text("[BOT]");
			} else if (!info.friendsID) {
				ImGui::Text("Unknown");
			} else {
				const auto& d = backpacktf::get_data(info.friendsID);
				if (d.bad && not d.pending) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
					ImGui::Text("Error");
				} else if (d.pending) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
					ImGui::Text("Loading");
				} else if (d.no_value) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
					ImGui::Text("Private?");
				} else if (d.outdated_value) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
					ImGui::Text("$%.2f", d.value);
				} else {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
					ImGui::Text("$%.2f", d.value);
				}
				ImGui::PopStyleColor();
			}
			x += 80;
		}
		ImGui::SameLine(x);
		ImGui::PushItemWidth(200.0f);
		if (ImGui::ColorEdit3("", data.color)) {
			if (!data.color.r && !data.color.b && !data.color.g) {
				data.color = colors::empty;
			} else {
				data.color.a = 255.0f;
			}
		}
		x += 200;
		ImGui::PopItemWidth();
		ImGui::SameLine(x);
		if (ImGui::Button("Spectate")) {
			spectator_target = eid;
		}
	}
	ImGui::PopID();
}

void RenderPlayerlist() {
	if (!g_IEngine->IsInGame()) return;
	if (ImGui::Begin("Player List")) {
		ImGui::SetWindowSize(ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));
		std::vector<int> teammates {};
		for (int i = 1; i < 32; i++) {
			if (!g_Settings.bInvalid && (g_pPlayerResource->GetTeam(i) == LOCAL_E->m_iTeam)) {
				teammates.push_back(i);
				continue;
			}
			RenderPlayer(i);
		}
		for (auto i : teammates) {
			RenderPlayer(i);
		}
		ImGui::PopStyleVar();
	}
	ImGui::End();
}

}}
