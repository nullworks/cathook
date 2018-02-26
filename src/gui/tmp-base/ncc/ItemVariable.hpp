/*
 * ItemVariable.hpp
 *
 *  Created on: Mar 26, 2017
 *      Author: nullifiedcat
 */

#ifndef ITEMVARIABLE_HPP_
#define ITEMVARIABLE_HPP_

#include "Item.hpp"

class CatVar;

namespace menu { namespace ncc {

class ItemVariable : public Item {
public:
	ItemVariable(CatVar& variable);

	void Change(float amount);

	virtual void Update() override;
	virtual bool ConsumesKey(int key) override;
	virtual void OnMousePress() override;
	virtual void OnFocusLose() override;
	virtual void OnKeyPress(int key, bool repeat) override;
	virtual void Draw(int x, int y) override;
public:
	CatVar& catvar;
	bool capturing { false };
};

}}

#endif /* ITEMVARIABLE_HPP_ */
