/*
 * CDropdownList.h
 *
 *  Created on: Jan 31, 2017
 *      Author: nullifiedcat
 */

#ifndef CDROPDOWNLIST_H_
#define CDROPDOWNLIST_H_

#include "CBaseContainer.h"

class CDropdown;
class CDropdownEntry;

class CDropdownList : public CBaseContainer {
public:
	CDropdownList(std::string name = "unnamed", CDropdown* menu = nullptr, int offset = 0);
	~CDropdownList();

	virtual void Draw(int x, int y);
	virtual void MoveChildren();
	inline virtual void SortByZIndex() override {};
	inline virtual bool DoesStealFocus() { return false; }

	void AddEntry(std::string name);
	void SetValue(int value);

	CDropdown* m_pMenu;
	std::vector<CDropdownEntry*> m_entries;
};

#endif /* CDROPDOWNLIST_H_ */
