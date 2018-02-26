/*
 * ItemTitle.hpp
 *
 *  Created on: Mar 26, 2017
 *      Author: nullifiedcat
 */

#ifndef ITEMTITLE_HPP_
#define ITEMTITLE_HPP_

#include "Item.hpp"

namespace menu { namespace ncc {

class ItemTitle : public Item {
public:
	ItemTitle(std::string title);

	virtual void Draw(int x, int y) override;
public:
	const std::string title;
};

}}

#endif /* ITEMTITLE_HPP_ */
