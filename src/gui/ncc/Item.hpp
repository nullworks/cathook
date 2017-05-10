/*
 * Item.hpp
 *
 *  Created on: Mar 26, 2017
 *      Author: nullifiedcat
 */

#ifndef ITEM_HPP_
#define ITEM_HPP_

#include "../CBaseWidget.h"

namespace menu { namespace ncc {

class Item : public CBaseWidget {
public:
	constexpr static int psize_x = 220;
	constexpr static int psize_y = 15;

	static int size_x;
	static int size_y;

	Item(std::string name = "ncc_menu_item");

	virtual void Draw(int x, int y) override;
	virtual void HandleCustomEvent(KeyValues* event) override;
};

}}

#endif /* ITEM_HPP_ */
