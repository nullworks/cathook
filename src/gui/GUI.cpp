/*
 * GUI.cpp
 *
 *  Created on: Nov 17, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"

namespace gui
{

void init()
{
    cmdui::init();
}

void update()
{
    cmdui::update();
}

void draw()
{
    cmdui::draw();
}
}

void GUIVisibleCallback(IConVar* var, const char* pOldValue, float flOldValue) {
	g_IInputSystem->SetCursorPosition(draw::width / 2, draw::height / 2);
	g_ISurface->SetCursor(vgui::CursorCode::dc_none);
	if (GUI) {
		g_ISurface->UnlockCursor();
		g_ISurface->SetCursorAlwaysVisible(true);
	}
    else
    {
		g_ISurface->LockCursor();
		g_ISurface->SetCursorAlwaysVisible(false);
	}
}

CatGUI::CatGUI() {}

CatGUI::~CatGUI() {}

void CatGUI::Setup() {
	GUI.OnRegister([](CatVar* var) {
		var->convar->InstallChangeCallback(GUIVisibleCallback);
	});
}

CatGUI* g_pGUI = 0;