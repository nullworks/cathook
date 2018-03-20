/*
 * GUI.hpp
 *
 *  Created on: Nov 14, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "cmdui/cmdui.hpp"

namespace gui
{

void init();
void update();
void draw();
}

class CatGUI {
public:
	CatGUI();
	~CatGUI();
	void Setup();
};

extern CatGUI* g_pGUI;