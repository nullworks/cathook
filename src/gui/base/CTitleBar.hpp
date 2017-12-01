
/*
 * TitleBar.h
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "CBaseWidget.hpp"

namespace gui { namespace base {

class CTitleBar : public CBaseWidget {
public:
	CTitleBar(std::string _title, IWidget* _parent);

	virtual void Draw();
	virtual void Update();

	std::string title;
	int drag_stage = 0;
	std::pair<int, int> last_mouse = std::make_pair(0, 0);
};

}}
