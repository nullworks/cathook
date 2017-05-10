/*
 * Tooltip.hpp
 *
 *  Created on: Mar 27, 2017
 *      Author: nullifiedcat
 */

#ifndef TOOLTIP_HPP_
#define TOOLTIP_HPP_

#include "../CTextLabel.h"

#include "../../common.h"

namespace menu { namespace ncc {

class Tooltip : public CTextLabel {
public:
	Tooltip();

	virtual void Draw(int x, int  y) override;
	virtual void HandleCustomEvent(KeyValues* event) override;
	inline virtual PositionMode GetPositionMode() override { return PositionMode::FLOATING; }
};

}}

#endif /* TOOLTIP_HPP_ */
