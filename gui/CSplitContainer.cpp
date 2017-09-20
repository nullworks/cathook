/*
 * CSplitContainer.cpp
 *
 *  Created on: Jan 28, 2017
 *      Author: nullifiedcat
 */

#include "CSplitContainer.h"
#include "../common.h"

CSplitContainer::CSplitContainer(std::string name, IWidget* parent) : CBaseContainer(name, parent) {}

void CSplitContainer::MoveChildren() {
	auto newsize = std::make_pair(GetMaxSize().first, 0);
	auto size = GetSize();
	if (!ChildCount()) return;
	int width = ((size.first - 4) / ChildCount()) - 2; // TODO padding!
	for (int i = 0; i < ChildCount(); i++) {
		auto child = ChildByIndex(i);
		child->SetOffset(2 + i * width, 2);
		child->SetMaxSize(width, -1);
		auto csize = child->GetSize();
		if (csize.second + 2 > newsize.second) newsize.second = csize.second + 2;
	}
	SetSize(-1, newsize.second);
}
