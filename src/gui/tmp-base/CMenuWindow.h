/*
 * CMenuWindow.h
 *
 *  Created on: Feb 3, 2017
 *      Author: nullifiedcat
 */

#ifndef CMENUWINDOW_H_
#define CMENUWINDOW_H_

#include "CBaseWindow.h"

class CMenuList;
class CMenuContainer;
class CTitleBar;

class CMenuWindow : public CBaseWindow {
public:
	CMenuWindow(std::string name, IWidget* parent);

	void AddElements();

	void SelectTab(std::string tab);
	void AddTab(std::string tab, std::string name);
	CMenuContainer* GetTab(std::string tab);

	virtual void MoveChildren() override;

	CMenuList* m_pList;
	CTitleBar* m_pTitle;
	CMenuContainer* m_pActiveTab;
};


#endif /* CMENUWINDOW_H_ */
