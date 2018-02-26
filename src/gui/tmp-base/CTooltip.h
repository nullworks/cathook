/*
 * CTooltip.h
 *
 *  Created on: Jan 28, 2017
 *      Author: nullifiedcat
 */

#ifndef CTOOLTIP_H_
#define CTOOLTIP_H_

#include "CTextLabel.h"

class CTooltip : public CTextLabel {
public:
	CTooltip(IWidget* parent = nullptr);

	virtual void Draw(int x, int y);
	inline virtual PositionMode GetPositionMode() { return PositionMode::FLOATING; }
};

#endif /* CTOOLTIP_H_ */
