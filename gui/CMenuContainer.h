/*
 * CMenuContainer.h
 *
 *  Created on: Feb 3, 2017
 *      Author: nullifiedcat
 */

#ifndef CMENUCONTAINER_H_
#define CMENUCONTAINER_H_

#include "CBaseContainer.h"

class CMenuWindow;

class CMenuContainer : public CBaseContainer {
public:
	CMenuContainer(std::string name, CMenuWindow* parent);

	virtual inline void SortByZIndex() override {};
	virtual void MoveChildren() override;
	virtual void Draw(int x, int y) override;
};

#endif /* CMENUCONTAINER_H_ */
