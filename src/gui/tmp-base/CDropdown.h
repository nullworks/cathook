/*
 * CDropdown.h
 *
 *  Created on: Jan 31, 2017
 *      Author: nullifiedcat
 */

#ifndef CDROPDOWN_H_
#define CDROPDOWN_H_

#include "CBaseButton.h"
#include "CDropdownList.h"

class CDropdown;

typedef std::function<void(CDropdown*, int)> DropdownCallbackFn_t;

class CDropdown : public CBaseButton {
public:
	CDropdown(std::string name = "unnamed", IWidget* parent = nullptr);
	~CDropdown();

	void AddValue(std::string);
	int ValueCount();
	std::string ValueName(int idx);
	void SetValue(int value);
	void SetValueInternal(int value);
	int  Value();

	void ShowList();
	void SetCallback(DropdownCallbackFn_t callback);

	virtual void Draw(int x, int y);
	virtual void OnFocusLose();

	DropdownCallbackFn_t m_pDropdownCallback;
	CDropdownList* list;
	std::vector<std::string> m_values;
};


#endif /* CDROPDOWN_H_ */
