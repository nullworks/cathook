/*
 * TitleBar.h
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#ifndef CCTITLEBAR_H_
#define CCTITLEBAR_H_

#include "CBaseWidget.h"

#define TITLEBAR_PADDING_W 2
#define TITLEBAR_PADDING_H 1

class CTitleBar : public CBaseWidget {
public:
	CTitleBar(IWidget* parent, std::string title);

	virtual void Draw(int x, int y);
	virtual void Update();

	std::string m_strTitle;
	int m_iDraggingStage;
	int m_nLastX;
	int m_nLastY;
};

#endif /* CCTITLEBAR_H_ */
