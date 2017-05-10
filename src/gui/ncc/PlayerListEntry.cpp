/*
 * PlayerListEntry.cpp
 *
 *  Created on: Apr 21, 2017
 *      Author: nullifiedcat
 */

#include "PlayerListEntry.hpp"
#include "../../helpers.h"

namespace menu { namespace ncc {

using namespace entrysubs;

void PlayerListEntry::Draw(int x, int y) {
	CBaseContainer::Draw(x, y);
}

void PlayerListEntry::Update() {
	if (idx) {
		SubBase* uid = dynamic_cast<SubBase*>(ChildByIndex(1));
		SubBase* steamid = dynamic_cast<SubBase*>(ChildByIndex(2));
		SubBase* name = dynamic_cast<SubBase*>(ChildByIndex(3));
		SubBase* clazz = dynamic_cast<SubBase*>(ChildByIndex(4));
		if (uid && steamid && name && clazz) {
			player_info_s info;
			bool success = g_IEngine->GetPlayerInfo(idx, &info);
			if (success) {
				uid->text = format(info.userID);
				steamid->text = format(info.friendsID);
				char safename[32];
				for (int i = 0, j = 0; i < 32; i++) {
					if (info.name[i] == 0) {
						safename[j] = 0;
						break;
					}
					if (info.name[i] == '\n') continue;
					safename[j++] = info.name[i];
				}
				name->text = format(safename);
				int iclazz = 0;
				if (TF) iclazz = g_pPlayerResource->GetClass(ENTITY(idx));
				int team = g_pPlayerResource->GetTeam(idx);
				clazz->color_bg = 0;
				if (idx != g_IEngine->GetLocalPlayer()) {
					if (team == TEAM_RED) {
						clazz->color_bg = colors::red;
					} else if (team == TEAM_BLU) {
						clazz->color_bg = colors::blu;
					}
				}
				if (iclazz && iclazz < 10) {
					clazz->text = classes[iclazz - 1];
				} else {
					clazz->text = "N/A";
				}
				Show();
			} else {
				uid->text = "N/A";
				steamid->text = "N/A";
				name->text = "N/A";
				clazz->text = "N/A";
				Hide();
			}
		} else {
			Hide();
		}
	}
	CBaseContainer::Update();
}

void PlayerListEntry::MoveChildren() {
	int it = 1;
	for (int i = 0; i < ChildCount() && i < (sizeof(size_table) / sizeof(int)); i++) {
		IWidget* child = ChildByIndex(i);
		child->SetOffset(it, 0);
		child->SetSize(size_table[i] * (float)scale, Item::size_y + 1);
		it += (size_table[i] + 1) * (float)scale;
	}
	SetSize(it, Item::size_y + 1);
}

void PlayerListEntry::OnKeyPress(ButtonCode_t key, bool repeat) {
	if (GetHoveredChild()) GetHoveredChild()->OnKeyPress(key, repeat);
}

PlayerListEntry::PlayerListEntry(int eid) : CBaseContainer(), idx(eid) {
	// If IDX == 0, this is a title.
	if (!idx) {
		AddChild(new SubTitle(*this, "EID"));
		AddChild(new SubTitle(*this, "UID"));
		AddChild(new SubTitle(*this, "SteamID"));
		AddChild(new SubTitle(*this, "Name"));
		AddChild(new SubTitle(*this, "Class"));
		AddChild(new SubTitle(*this, "State"));
		AddChild(new SubTitle(*this, "R"));
		AddChild(new SubTitle(*this, "G"));
		AddChild(new SubTitle(*this, "B"));
		Show();
	} else {
		Hide();
		SubBase* eid = new SubBase(*this);
		eid->text = format(idx);
		AddChild(eid); // EID
		AddChild(new SubBase(*this)); // UID
		AddChild(new SubBase(*this)); // SteamID
		AddChild(new SubBase(*this)); // Name
		AddChild(new SubBase(*this)); // Class
		AddChild(new SubState(*this));
		AddChild(new SubColorComponent(*this, SubColorComponent::Component::R));
		AddChild(new SubColorComponent(*this, SubColorComponent::Component::G));
		AddChild(new SubColorComponent(*this, SubColorComponent::Component::B));
	}
}

SubBase::SubBase(PlayerListEntry& base) : parent(base), CBaseWidget() {}

void SubBase::Draw(int x, int y) {
	if (color_bg) {
		const auto& size = GetSize();
		draw::DrawRect(x, y, size.first, size.second, color_bg);
	}
	draw::String(menu::ncc::font_item, x + 2, y + 2, color_fg ? color_fg : colors::white, 2, text);
}

SubTitle::SubTitle(PlayerListEntry& parent, const std::string& title) : SubBase(parent) {
	text = title;
}

void SubTitle::Draw(int x, int y) {
	draw::String(menu::ncc::font_title, x + 2, y + 2, colors::white, 2, text);
}

SubColorComponent::SubColorComponent(PlayerListEntry& parent, Component component) : SubBase(parent), component(component) {}

void SubColorComponent::Update() {
	player_info_s info;
	bool success = g_IEngine->GetPlayerInfo(parent.idx, &info);
	if (success) {
		playerlist::userdata& data = playerlist::AccessData(info.friendsID);
		color_bg = data.color;
	} else {
		color_bg = 0;
	}
	text = format((int)(reinterpret_cast<unsigned char*>(&color_bg)[static_cast<int>(component)]));
}

bool SubColorComponent::ConsumesKey(ButtonCode_t key) {
	return key == MOUSE_WHEEL_DOWN || key == MOUSE_WHEEL_UP;
}
void SubColorComponent::OnKeyPress(ButtonCode_t key, bool repeat) {
	player_info_s info;
	bool success = g_IEngine->GetPlayerInfo(parent.idx, &info);
	if (!success) return;
	playerlist::userdata& data = playerlist::AccessData(info.friendsID);
	int change = 0;
	if (key == MOUSE_WHEEL_UP) {
		if (g_IInputSystem->IsButtonDown(KEY_LSHIFT)) {
			change = 50;
		} else if (g_IInputSystem->IsButtonDown(KEY_LCONTROL)) {
			change = 10;
		} else {
			change = 1;
		}
	} else if (key == MOUSE_WHEEL_DOWN) {
		if (g_IInputSystem->IsButtonDown(KEY_LSHIFT)) {
			change = -50;
		} else if (g_IInputSystem->IsButtonDown(KEY_LCONTROL)) {
			change = -10;
		} else {
			change = -1;
		}
	}
	int result = _clamp(0, 255, (int)(reinterpret_cast<unsigned char*>(&data.color)[static_cast<int>(component)]) + change);
	(reinterpret_cast<unsigned char*>(&data.color)[static_cast<int>(component)]) = (unsigned char)result;
	if (!(data.color & 0x00FFFFFF)) {
		data.color = 0;
	} else {
		data.color |= 0xFF000000;
	}
}

void SubState::Update() {
	player_info_s info;
	bool success = g_IEngine->GetPlayerInfo(parent.idx, &info);
	if (!success) {
		text = "N/A";
		return;
	}
	playerlist::userdata& data = playerlist::AccessData(info.friendsID);
	if (parent.idx == g_IEngine->GetLocalPlayer()) {
		text = "Local Player";
		return;
	}
	const int state = static_cast<int>(data.state);
	color_bg = playerlist::Color(info.friendsID);
	if (state >= 0 && state <= static_cast<int>(playerlist::k_EState::STATE_LAST)) {
		text = playerlist::k_Names[state];
	} else {
		text = "N/A";
	}
}

bool SubState::ConsumesKey(ButtonCode_t key) {
	return key == MOUSE_WHEEL_DOWN || key == MOUSE_WHEEL_UP;
}
void SubState::OnKeyPress(ButtonCode_t key, bool repeat) {
	if (parent.idx == g_IEngine->GetLocalPlayer()) return;
	player_info_s info;
	bool success = g_IEngine->GetPlayerInfo(parent.idx, &info);
	if (!success) return;
	playerlist::userdata& data = playerlist::AccessData(info.friendsID);
	int state = static_cast<int>(data.state);
	int change = 0;
	if (key == MOUSE_WHEEL_UP) {
		change = 1;
	} else if (key == MOUSE_WHEEL_DOWN) {
		change = -1;
	}
	int result = _clamp(0, static_cast<int>(playerlist::k_EState::STATE_LAST), state + change);
	data.state = static_cast<playerlist::k_EState>(result);
}

}}
