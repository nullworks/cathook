
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
	CTitleBar(const char* _title, IWidget* _parent);

	virtual void Draw();
	virtual void Update();

	const std::string title;
	bool dragging = false;
	std::pair<int, int> last_mouse;
};

}}
