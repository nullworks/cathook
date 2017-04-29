/*
 * GUI.cpp
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#include "GUI.h"
#include "IWidget.h"
#include "RootWindow.h"
#include "CTooltip.h"

#include "../common.h"
#include "../sdk.h"



void GUIVisibleCallback(IConVar* var, const char* pOldValue, float flOldValue) {
	g_IInputSystem->SetCursorPosition(draw::width / 2, draw::height / 2);
	g_ISurface->SetCursor(vgui::CursorCode::dc_none);
	g_ISurface->SetCursorAlwaysVisible(false);
	//g_IMatSystemSurface->SetCursorAlwaysVisible(false);
	if (gui_visible) {
		g_ISurface->UnlockCursor();
		//g_IMatSystemSurface->UnlockCursor();
	} else {
		g_ISurface->LockCursor();
		//g_IMatSystemSurface->LockCursor();
	}
}

CatVar gui_visible(CV_SWITCH, "gui_visible", "0", "GUI Active", "GUI switch (bind it to a key!)");
CatVar gui_draw_bounds(CV_SWITCH, "gui_bounds", "0", "Draw Bounds", "Draw GUI elements' bounding boxes");
//CatVar gui_nullcore(CV_SWITCH, "gui_nullcore", "1", "NullCore GUI", "Use NullCoreCheat GUI");

CatGUI::CatGUI() {
	root_nullcore = nullptr;
	m_pRootWindow = 0;
}

CatGUI::~CatGUI() {
	delete root_nullcore;
	delete m_pRootWindow;
}

bool CatGUI::Visible() {
	return gui_visible;
}

CatVar gui_color_r(CV_INT, "gui_color_r", "255", "Main GUI color (red)", "Defines red component of main gui color", 0, 255);
CatVar gui_color_g(CV_INT, "gui_color_g", "105", "Main GUI color (green)", "Defines green component of main gui color", 0, 255);
CatVar gui_color_b(CV_INT, "gui_color_b", "180", "Main GUI color (blue)", "Defines blue component of main gui color", 0, 255);

static CatVar gui_rainbow(CV_SWITCH, "gui_rainbow", "0", "Rainbow GUI", "RGB all the things!!!");
int GUIColor() {
	return gui_rainbow ? colors::RainbowCurrent() : colors::Create((int)gui_color_r, (int)gui_color_g, (int)gui_color_b, 255);
}

void CatGUI::Setup() {
	m_pRootWindow = new RootWindow();
	m_pRootWindow->Setup();
	menu::ncc::Init();
	root_nullcore = new menu::ncc::Root();
	root_nullcore->Setup();
	gui_visible.OnRegister([](CatVar* var) {
		var->convar->InstallChangeCallback(GUIVisibleCallback);
	});
}

void CatGUI::ShowTooltip(std::string text) {
	m_pTooltip->SetText(text);
	m_pTooltip->SetOffset(m_iMouseX + 5, m_iMouseY + 5);
	m_pTooltip->Show();
	m_bShowTooltip = true;
}

void CatGUI::Update() {
	try {
		CBaseWindow* root = gui_nullcore ? dynamic_cast<CBaseWindow*>(root_nullcore) : dynamic_cast<CBaseWindow*>(m_pRootWindow);
		if (gui_nullcore) m_pRootWindow->Hide();
		else root_nullcore->Hide();
		m_bShowTooltip = false;
		int new_scroll = g_IInputSystem->GetAnalogValue(AnalogCode_t::MOUSE_WHEEL);
		//logging::Info("scroll: %i", new_scroll);
		if (last_scroll_value < new_scroll) {
			// Scrolled up
			m_bPressedState[ButtonCode_t::MOUSE_WHEEL_DOWN] = false;
			m_bPressedState[ButtonCode_t::MOUSE_WHEEL_UP] = true;
		} else if (last_scroll_value > new_scroll) {
			// Scrolled down
			m_bPressedState[ButtonCode_t::MOUSE_WHEEL_DOWN] = true;
			m_bPressedState[ButtonCode_t::MOUSE_WHEEL_UP] = false;
		} else {
			// Didn't scroll
			m_bPressedState[ButtonCode_t::MOUSE_WHEEL_DOWN] = false;
			m_bPressedState[ButtonCode_t::MOUSE_WHEEL_UP] = false;
		}

		last_scroll_value = new_scroll;
		for (int i = 0; i < ButtonCode_t::BUTTON_CODE_COUNT; i++) {
			bool down = false, changed = false;;
			if (i != ButtonCode_t::MOUSE_WHEEL_DOWN && i != ButtonCode_t::MOUSE_WHEEL_UP) {
				down = g_IInputSystem->IsButtonDown((ButtonCode_t)(i));
				changed = m_bPressedState[i] != down;
			} else {
				down = m_bPressedState[i];
				changed = down;
			}
			if (changed && down) m_iPressedFrame[i] = g_GlobalVars->framecount;
			m_bPressedState[i] = down;
			if (m_bKeysInit) {
				if (changed) {
					//logging::Info("Key %i changed! Now %i.", i, down);
					if (i == ButtonCode_t::MOUSE_LEFT) {
						if (Visible()) {
							if (down) root->OnMousePress();
							else root->OnMouseRelease();
						}
					} else {
						if (i == ButtonCode_t::KEY_INSERT && down) {
							gui_visible = !gui_visible;
						}
						if (Visible()) {
							if (down) root->OnKeyPress((ButtonCode_t)i, false);
							else root->OnKeyRelease((ButtonCode_t)i);
						}
					}
				} else {
					if (down) {
						int frame = g_GlobalVars->framecount - m_iPressedFrame[i];
						bool shouldrepeat = false;
						if (frame) {
							if (frame > 150) {
								if (frame > 400) {
									if (frame % 30 == 0) shouldrepeat = true;
								} else {
									if (frame % 80 == 0) shouldrepeat = true;
								}
							}
						}
						if (Visible()) {
							if (shouldrepeat) root->OnKeyPress((ButtonCode_t)i, true);
						}
					}
				}
			}
		}

		int nx = g_IInputSystem->GetAnalogValue(AnalogCode_t::MOUSE_X);
		int ny = g_IInputSystem->GetAnalogValue(AnalogCode_t::MOUSE_Y);

		mouse_dx = nx - m_iMouseX;
		mouse_dy = ny - m_iMouseY;

		m_iMouseX = nx;
		m_iMouseY = ny;

		if (!m_bKeysInit) m_bKeysInit = 1;
		if (!root->IsVisible())
			root->Show();
		root->Update();
		if (!m_bShowTooltip && m_pTooltip->IsVisible()) m_pTooltip->Hide();
		root->Draw(0, 0);
		if (Visible()) {
			draw::DrawRect(m_iMouseX - 5, m_iMouseY - 5, 10, 10, colors::Transparent(colors::white));
			draw::OutlineRect(m_iMouseX - 5, m_iMouseY - 5, 10, 10, GUIColor());
		}
		if (gui_draw_bounds) {
			root->DrawBounds(0, 0);
		}
		/*if (gui_visible) {
			if (!root->IsVisible())
				root->Show();
			root->Update();
			if (!m_bShowTooltip && m_pTooltip->IsVisible()) m_pTooltip->Hide();
			root->Draw(0, 0);
			draw::DrawRect(m_iMouseX - 5, m_iMouseY - 5, 10, 10, colors::Transparent(colors::white));
			draw::OutlineRect(m_iMouseX - 5, m_iMouseY - 5, 10, 10, GUIColor());
			if (gui_draw_bounds) {
				root->DrawBounds(0, 0);
			}
		} else {
			if (root->IsVisible())
				root->Hide();
		}*/
	} catch (std::exception& ex) {
		logging::Info("ERROR: %s", ex.what());
	}

}

bool CatGUI::ConsumesKey(ButtonCode_t key) {
	CBaseWindow* root = gui_nullcore ? dynamic_cast<CBaseWindow*>(root_nullcore) : dynamic_cast<CBaseWindow*>(m_pRootWindow);
	if (root->IsVisible())
		return root->ConsumesKey(key);
	else return false;
}

RootWindow* CatGUI::GetRootWindow() {
	return m_pRootWindow;
}

CatGUI* g_pGUI = 0;
