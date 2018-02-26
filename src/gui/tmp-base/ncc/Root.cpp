/*
 * Root.cpp
 *
 *  Created on: Mar 26, 2017
 *      Author: nullifiedcat
 */

#include "../../../framework/input.hpp"
#include "Root.hpp"
#include "Menu.hpp"
#include "Tooltip.hpp"

namespace menu { namespace ncc {

Root::Root() : CBaseWindow("root_nullcore", nullptr) {

}

void Root::Update() {
	tooltip->Hide();
	CBaseWindow::Update();
	SetMaxSize(input::bounds.first, input::bounds.second);
}

void Root::Draw(int x, int y) {
	if (tooltip->IsVisible()) {
		tooltip->SetOffset(g_pGUI->m_iMouseX + 24, g_pGUI->m_iMouseY + 8);
	}
	CBaseContainer::Draw(x, y);
}

void Root::Setup() {
	tooltip = new Tooltip();
	AddChild(tooltip);
	AddChild(menu::ncc::List::GetMainList());
	menu::ncc::List::GetMainList()->ChildByIndex(0)->Props()->SetBool("brackets3", true);
	menu::ncc::List::GetMainList()->Show();
	menu::ncc::List::GetMainList()->SetOffset(input::bounds.first / 2, input::bounds.second / 2);
}

void Root::OnKeyPress(int key, bool repeat) {
	if (GetHoveredChild()) GetHoveredChild()->OnKeyPress(key, repeat);
}

}}
