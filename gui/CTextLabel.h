/*
 * CTextLabel.h
 *
 *  Created on: Jan 26, 2017
 *      Author: nullifiedcat
 */

#ifndef CTEXTLABEL_H_
#define CTEXTLABEL_H_

#include "CBaseWidget.h"

#include "../fixsdk.h"
#include "Color.h"

class CTextLabel : public CBaseWidget {
public:
	CTextLabel(std::string name = "unnamed", IWidget* parent = nullptr, std::string text = "", bool centered = false);

	void SetText(std::string text);
	std::string GetText();
	void SetPadding(int x, int y);
	void SetAutoSize(bool autosize);
	void SetCentered(bool centered);

	virtual void Draw(int x, int y);
};

#endif /* CTEXTLABEL_H_ */
