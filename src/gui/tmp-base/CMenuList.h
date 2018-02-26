/*
 * CMenuList.h
 *
 *  Created on: Feb 3, 2017
 *      Author: nullifiedcat
 */

#ifndef CMENULIST_H_
#define CMENULIST_H_

#include "CBaseContainer.h"

class CMenuWindow;
class CMenuContainer;
class CMenuListEntry;

class CMenuList : public CBaseContainer {
public:
	CMenuList(std::string name, CMenuWindow* parent);

	void SelectEntry(std::string id);
	void AddEntry(std::string id, std::string name);

	virtual void MoveChildren() override;

	CMenuListEntry* m_pSelected;
};

#endif /* CMENULIST_H_ */
