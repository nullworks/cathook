/*
 * CTextInput.h
 *
 *  Created on: Jan 26, 2017
 *      Author: nullifiedcat
 */

#ifndef CTEXTINPUT_H_
#define CTEXTINPUT_H_

#include "CBaseWidget.h"

class CTextInput;

typedef std::function<void(CTextInput*, std::string, std::string)> TextInputCallbackFn_t;

class CTextInput : public CBaseWidget {
public:
	CTextInput(std::string name = "unnamed", IWidget* parent = nullptr);

	virtual void OnKeyPress(ButtonCode_t key, bool repeat);
	virtual void Draw(int x, int y);
	virtual bool ConsumesKey(ButtonCode_t key);

	void PutChar(char ch);
	void SetLength(int newlength);
	void SetMaxWidth(int width);
	void SetCallback(TextInputCallbackFn_t callback);
	std::string Value();
	void SetValue(std::string value);

	TextInputCallbackFn_t m_pCallback;
};

#endif /* CTEXTINPUT_H_ */
