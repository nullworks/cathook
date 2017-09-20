/*
 * CKey.h
 *
 *  Created on: Feb 1, 2017
 *      Author: nullifiedcat
 */

#ifndef CKEYINPUT_H_
#define CKEYINPUT_H_

#include "CBaseWidget.h"

class CKeyInput;

typedef std::function<void(CKeyInput*, ButtonCode_t)> KeyInputCallbackFn_t;

class CKeyInput : public CBaseWidget {
public:
	CKeyInput(std::string name = "unnamed", IWidget* parent = nullptr);

	ButtonCode_t Value();
	void SetValue(int value);
	void SetCallback(KeyInputCallbackFn_t callback);

	KeyInputCallbackFn_t m_pCallback;

	virtual void OnKeyPress(ButtonCode_t key, bool repeat) override;
	virtual void Draw(int x, int y) override;
	virtual void OnMousePress() override;
	virtual void OnFocusLose() override;
	virtual bool ConsumesKey(ButtonCode_t key) override;
};

#endif /* CKEYINPUT_H_ */
