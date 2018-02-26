/*
 * GUI.h
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#ifndef GUI_H_
#define GUI_H_

class IWidget;
class CatVar;

#include <string>

#include "../../framework/input.hpp"

#include "ncc/Root.hpp"
#include "ncc/Menu.hpp"

class CTooltip;
class RootWindow;

extern CatVarInt gui_color_r;
extern CatVarInt gui_color_g;
extern CatVarInt gui_color_b;
Color GUIColor();

extern CatVarBool gui_visible;
extern CatVarBool gui_draw_bounds;
constexpr bool gui_nullcore = true;

class CatGUI {
public:
	CatGUI();
	~CatGUI();

	bool Visible();
	void Update();
	void Setup();
	RootWindow* GetRootWindow();
	bool ConsumesKey(int key);

	void ShowTooltip(std::string text);

	CTooltip* m_pTooltip;
	RootWindow* m_pRootWindow;

	// TODO NullCore tooltip
	menu::ncc::Root* root_nullcore;

	int  last_scroll_value;
	bool m_bShowTooltip;
	bool m_bConsumeKeys;
	bool m_bKeysInit;
	// used to store what frame we are on
	unsigned long frame_count = 0;
	// Used to store our previous presses and times they were pressed
	bool pressed_state[CATKEY_COUNT];
	int  pressed_frame[CATKEY_COUNT];
	int  m_iMouseX;
	int  m_iMouseY;
	int  mouse_dx;
	int  mouse_dy;
};

extern CatGUI* g_pGUI;

#endif /* GUI_H_ */
