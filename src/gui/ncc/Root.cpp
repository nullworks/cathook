/*
 * Root.cpp
 *
 *  Created on: Mar 26, 2017
 *      Author: nullifiedcat
 */

#include "Root.hpp"
#include "Menu.hpp"
#include "Tooltip.hpp"
#include "Radar.hpp"
#include "../../common.h"

namespace menu { namespace ncc {

Texture logo_texture(&_binary_logo_start, 768, 384);

Root::Root() : CBaseWindow("root_nullcore", nullptr) {
	SetMaxSize(draw::width, draw::height);
}

void Root::Update() {
	tooltip->Hide();
	CBaseWindow::Update();
}

void Root::Draw(int x, int y) {
	if (tooltip->IsVisible()) {
		tooltip->SetOffset(g_pGUI->m_iMouseX + 24, g_pGUI->m_iMouseY + 8);
	}
	CBaseContainer::Draw(x, y);
}

void Root::Setup() {
	tooltip = new Tooltip();
	Logo* logo = new Logo();
	logo->SetOffset(draw::width / 2 - 288, 25);
	AddChild(new Background());
	AddChild(logo);
	AddChild(tooltip);
	AddChild(&menu::ncc::MainList());
	menu::ncc::MainList().ChildByIndex(0)->Props()->SetBool("brackets3", true);
	AddChild(new Radar());
	menu::ncc::MainList().Show();
	menu::ncc::MainList().SetOffset(draw::width / 2, draw::height / 2);
	PlayerList* pl = new PlayerList();
	pl->SetOffset(200, 200);
	AddChild(pl);
}

void Root::OnKeyPress(ButtonCode_t key, bool repeat) {
	if (GetHoveredChild()) GetHoveredChild()->OnKeyPress(key, repeat);
}

}}
