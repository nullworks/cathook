
/*
 * TitleBar.h
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "CBaseWidget.h"

class CTitleBar : public CBaseWidget {
public:
	CTitleBar(std::string title, IWidget* parent);

	virtual void Draw();
	virtual void Update();

	std::string title;
	int drag_stage = 0;
	std::pair<int, int> last_mouse(0, 0);
};

