/*
 * Item.cpp
 *
 *  Created on: Mar 26, 2017
 *      Author: nullifiedcat
 */

#include "Item.hpp"
#include "Menu.hpp"

namespace menu { namespace ncc {

int Item::size_x = 0;
int Item::size_y = 0;

Item::Item(std::string name) : CBaseWidget(name, nullptr) {
	SetSize(psize_x, psize_y);
	SetMaxSize(psize_x, psize_y);
}

void Item::Draw(int x, int y) {
	const auto& size = GetSize();
	//draw::DrawRect(x, y, size.first, size.second, colors::red);
	draw::RectFilled(x, y, size.first, size.second, Color(0, 0, 0, 77));
	if (IsHovered()) {
		draw::RectFilled(x, y, size.first, size.second, colors::Transparent(GUIColor(), 0.32f));
	}
}

void Item::HandleCustomEvent(KeyValues* event) {
	//if (!strcmp(event->GetName(), "scale_update")) {
		size_x = psize_x * (float)scale;
		size_y = psize_y * (float)scale;
		SetSize(size_x, size_y);
		SetMaxSize(size_x, size_y);
	//}
}

}}
