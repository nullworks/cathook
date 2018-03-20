/*
 *
 *	A widget with children, who are forced into little lines.
 *  Children, Right.. Dress! Eyes.. Front! Stand at.. Ease!
 *
 */

#pragma once

#include <vector>

#include "CBaseParent.hpp"

namespace gui { namespace base {

class CBaseContainer : public CBaseParent {
public:
	CBaseContainer(std::string name, std::string tooltip = "");

	// General functions
	virtual void UpdatePositioning();

	// Child related util
	virtual void MoveChildren();

	//Column things
	int columnSpacing = 4;
	int defaultColumnWidth=100;//TODO: CatVar?
	std::vector<int> columnWidth;
};

}}
