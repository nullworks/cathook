/*
 * CCVarContainer.h
 *
 *  Created on: Jan 28, 2017
 *      Author: nullifiedcat
 */

#ifndef CCVARCONTAINER_H_
#define CCVARCONTAINER_H_

#include "CBaseContainer.h"

class CatVar;
class CTextInput;
class CTextLabel;

class CCVarContainer : public CBaseContainer {
public:
	CCVarContainer(IWidget* parent, CatVar* var);

	virtual void MoveChildren() override;
	virtual void Update() override;

	CTextInput* m_pInput;
	IWidget* m_pControl;
	CTextLabel* m_pLabel;

	CatVar* m_pVar;
};

#endif /* CCVARCONTAINER_H_ */
