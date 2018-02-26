/*
 * Root.hpp
 *
 *  Created on: Mar 26, 2017
 *      Author: nullifiedcat
 */

#ifndef ROOT_HPP_
#define ROOT_HPP_

#include "../CBaseWindow.h"

namespace menu { namespace ncc {

class Root : public CBaseWindow {
public:
	Root();
	void Setup();
	virtual void Update() override;
	virtual void OnKeyPress(int key, bool repeat) override;
	virtual void Draw(int x, int y) override;
	inline virtual void MoveChildren() override {};
};

}}

#endif /* ROOT_HPP_ */
