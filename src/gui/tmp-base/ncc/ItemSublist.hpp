/*
 * ItemSublist.hpp
 *
 *  Created on: Mar 26, 2017
 *      Author: nullifiedcat
 */

#ifndef ITEMSUBLIST_HPP_
#define ITEMSUBLIST_HPP_

#include "Item.hpp"
#include "List.hpp"

class CatVar;

namespace menu { namespace ncc {

class List; // ????

class ItemSublist : public Item {
public:
	ItemSublist(std::string title, List* list);

	virtual void SetParent(IWidget*) override;
	virtual bool IsHovered() override;
	virtual void Update() override;
	virtual void Draw(int x, int y) override;
	virtual void OnKeyPress(int code, bool repeated) override;
	virtual void OnMouseEnter();
	virtual void OnMouseLeave();
public:
	List* const list;
	const std::string title;
};

}}

#endif /* ITEMSUBLIST_HPP_ */
