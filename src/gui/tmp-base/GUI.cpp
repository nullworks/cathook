/*
 * GUI.cpp
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#include "IWidget.h"
#include "RootWindow.h"
#include "CTooltip.h"

#include "GUI.h"

// Cursors are gay
/*void GUIVisibleCallback(IConVar* var, const char* pOldValue, float flOldValue) {
	g_IInputSystem->SetCursorPosition(draw::width / 2, draw::height / 2);
	g_ISurface->SetCursor(vgui::CursorCode::dc_none);
	//g_IMatSystemSurface->SetCursorAlwaysVisible(false);
	if (gui_visible) {
		g_ISurface->UnlockCursor();
		g_ISurface->SetCursorAlwaysVisible(true);
		//g_IMatSystemSurface->UnlockCursor();
	} else {
		g_ISurface->LockCursor();
		g_ISurface->SetCursorAlwaysVisible(false);
		//g_IMatSystemSurface->LockCursor();
	}
}*/

CatEnum gui_menu_menu_menu_menu_menu_menu_menu({"Visuals", "Menu"});
CatVarBool gui_visible(gui_menu_menu_menu_menu_menu_menu_menu, "gui_visible", false, "GUI Active", "GUI switch (bind it to a key!)");
CatVarBool gui_draw_bounds(gui_menu_menu_menu_menu_menu_menu_menu, "gui_bounds", false, "Draw Bounds", "Draw GUI elements' bounding boxes");
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

CatVarInt gui_color_r(gui_menu_menu_menu_menu_menu_menu_menu, "gui_color_r", 255, "Main GUI color (red)", "Defines red component of main gui color", 0, 255);
CatVarInt gui_color_g(gui_menu_menu_menu_menu_menu_menu_menu, "gui_color_g", 105, "Main GUI color (green)", "Defines green component of main gui color", 0, 255);
CatVarInt gui_color_b(gui_menu_menu_menu_menu_menu_menu_menu, "gui_color_b", 180, "Main GUI color (blue)", "Defines blue component of main gui color", 0, 255);

static CatVarBool gui_rainbow(gui_menu_menu_menu_menu_menu_menu_menu, "gui_rainbow", false, "Rainbow GUI", "RGB all the things!!!");
Color GUIColor() {
	return gui_rainbow ? colors::RainbowCurrent() : Color((int)gui_color_r, (int)gui_color_g, (int)gui_color_b, 255);
}

void CatGUI::Setup() {
	m_pRootWindow = new RootWindow();
	m_pRootWindow->Setup();
	menu::ncc::Init();
	root_nullcore = menu::ncc::root;
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
		// Input handler
		for (int i = 0; i < CATKEY_COUNT; i++) {

	    // Get whether key is down and whether key state changed from last tick
			const bool& down = input::pressed_buttons[i];
			bool changed = (i == CATKEY_M_WHEEL_DOWN || i == CATKEY_M_WHEEL_UP) ? down : pressed_state[i] != down; // If its a mouse wheel, we dont care about last state
			// Update last state and Tick key was pressed
			pressed_state[i] = down;
			if (changed && down) pressed_frame[i] = frame_count;

			// If our key has changed, we send out this keypress to the gui widgets
			if (changed) {
	  		// Send out a keypress to the gui
	  		if (down) {
	  			if (i == CATKEY_MOUSE_1)
	  				root->OnMousePress();
	  			else if (i == CATKEY_INSERT || i == CATKEY_F11)
	  				gui_visible = !gui_visible;
	  			else
	  				root->OnKeyPress(i, false);

	  		// For releases of keypresses
	  		} else {
	  			if (i == CATKEY_MOUSE_1)
	  				root->OnMouseRelease();
	  			else
	  				root->OnKeyRelease(i);
	  		}

	  	// If a key hasent changed, We can repeat the key pressed after a period of time
	  	} else if (down) {
	  		// Get how long tick was depressed
	  	  int frame = frame_count - pressed_frame[i];
	  		if (frame > 150) {
	  			// if frame was pressed for less than 400 ticks, we repeat every 80 ticks, otherwise we repeat every 30
	  			if ((frame > 400) ? frame % 30 == 0 : frame % 80 == 0) {
	  				// Send out a keypress to the gui with repeat true
	  				root->OnKeyPress(i, true);
	  			}
	  		}
	  	}
	  }

	  // Increse our framecount
	  frame_count++;

		int nx = input::mouse.first;
		int ny = input::mouse.second;
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
			draw::RectFilled(m_iMouseX - 5, m_iMouseY - 5, 10, 10, colors::Transparent(colors::white));
			draw::Rect(m_iMouseX - 5, m_iMouseY - 5, 10, 10, GUIColor());
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
		g_CatLogging.log("ERROR: %s", ex.what());
	}

}

bool CatGUI::ConsumesKey(int key) {
	CBaseWindow* root = gui_nullcore ? dynamic_cast<CBaseWindow*>(root_nullcore) : dynamic_cast<CBaseWindow*>(m_pRootWindow);
	if (root->IsVisible())
		return root->ConsumesKey(key);
	else return false;
}

RootWindow* CatGUI::GetRootWindow() {
	return m_pRootWindow;
}

CatGUI* g_pGUI = 0;
