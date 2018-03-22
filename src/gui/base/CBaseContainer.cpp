/*
 * CBaseContainer.cpp
 *
 *  Created on: Jan 31, 2017
 *      Author: nullifiedcat
 */

#include <cstring> // strcmp()
#include <algorithm> // std::sort, min, max

#include "../../util/logging.hpp"			// We log stuff
#include "../../framework/drawing.hpp" // Draw stuff
#include "../../util/colors.hpp" // Draw stuff
#include "../../framework/input.hpp"
#include "../gui.hpp"

#include "CBaseContainer.hpp"

namespace gui { namespace base {

using namespace gui;

// Constructor & destructor
CBaseContainer::CBaseContainer(std::string name, std::string tooltip) : CBaseParent(name, tooltip) {}

// General functions
void CBaseContainer::UpdatePositioning() {
	//Bypass CBaseParent's positioning, we don't want our children to be moved like that
	CBaseWidget::UpdatePositioning();
	MoveChildren();
}

bool CBaseContainer::OnBounds(std::pair<int,int> bounds){
	MoveChildren();
	CBaseParent::OnBounds(bounds);
}
void CBaseContainer::MoveChildren() {
	//TODO: Decipher MoveChildren, as written by nullcat back in 2017
	// Used space
	std::pair<int, int> space = std::make_pair(0,0);
	//TODO: Code recycling: Absolutes
	/*// Find a Bounding box around all of the absolutes
	for (auto c : children) {
		if (!c->IsVisible()) continue;
		// Check if not absolute
		if (c->position_mode != ABSOLUTE)
			continue;
		// If some our used space is less than the space taken by the widget, add it to used space.
		space.first = std::max(space.first, c->offset.first + c->size.first);
		space.second = std::max(space.second, c->offset.second + c->size.second);
	}*/
	
	// Organize our inlines
	int x = 0;
	auto bounds = input::GetBounds();
	//g_CatLogging.log("Bounds %d %d", bounds.first, bounds.second);
	if (bounds.first<10 || bounds.second<10){
		g_CatLogging.log("%s takes an early leave, the bounds (%d, %d) are too small",name.c_str(), bounds.first, bounds.second);
		return;
	}
	int longest_column_height = 0;//Cannot exceed bounds.second
	int column_height = space.second;//init'd to abs' height
	int column=0;
	int columnFirstIDX=0;
	if(columnWidth.size()==0){
		columnWidth.push_back(defaultColumnWidth);
	}
	//for (auto c : children) {
	for(int idx=0;idx<children.size();idx++){
		auto c = children[idx];
		// Check if visible
		if (!c->visible)
			continue;
		c->size.first=columnWidth[column];
		c->offset=std::make_pair(x, column_height);
		c->UpdatePositioning();
		if(c->size.first>columnWidth[column]){
			g_CatLogging.log("Resetting column %d because \"%s\" is %dpx wide, and this column is only %dpx wide!", column, c->name.c_str(), c->size.first, columnWidth[column]);
			columnWidth[column]=c->size.first;
			idx = columnFirstIDX-1;
			continue;//Increments idx
		}
		//Wrap to Next Column
		if(column_height+c->size.second>bounds.second){
			g_CatLogging.log("Finishing column %d because \"%s\" is %dpx tall, and only %dpx of space are left.", column, c->name.c_str(), c->size.second, bounds.second-column_height);
			//Check for the longest column
			longest_column_height=std::max(column_height,longest_column_height);
			//Move right
			x+=columnWidth[column]+columnSpacing;
			//If this is the first time this column has existed
			if(++column>=columnWidth.size()){
				//Give it a default width
				columnWidth.push_back(defaultColumnWidth);
			}
			//Reset to top of column
			column_height=space.second;
			columnFirstIDX=idx;
		}
		//Add to column
		column_height+=c->size.second;
	}
	//Check for longest column
	longest_column_height = std::max(column_height,longest_column_height);
	//Move right
	x+=columnWidth[column]+columnSpacing;
	//Set own size
	size = std::make_pair(x,longest_column_height);
	//Ensure we don't go off the bottom of the screen
	offset.second=std::min(offset.second,bounds.second+offset.second-global_pos.second-longest_column_height);
}

}}
