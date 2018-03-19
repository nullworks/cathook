
/*
 * TitleBar.h
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "CBaseWidget.hpp"

namespace gui { namespace base {

class CBaseTitleBar : public CBaseWidget {
public:
	CBaseTitleBar(const char* _title);

	virtual void Draw();
	virtual void Update();
	virtual void OnMousePress();

	const std::string title;
	std::pair<int, int> last_mouse;
};

}}
