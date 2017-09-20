/*
 * CBaseButton.h
 *
 *  Created on: Jan 26, 2017
 *      Author: nullifiedcat
 */

#ifndef CBASEBUTTON_H_
#define CBASEBUTTON_H_

#include "../beforecheaders.h"
#include <functional>
#include "../aftercheaders.h"

#include "CTextLabel.h"

#define BUTTON_PADDING_W 3
#define BUTTON_PADDING_H 2

class CBaseButton;

typedef std::function<void(CBaseButton*)> ButtonCallbackFn_t;

class CBaseButton : public CTextLabel {
public:
	CBaseButton(std::string name = "unnamed", IWidget* parent = nullptr, std::string text = "", ButtonCallbackFn_t callback = nullptr);

	virtual void Draw(int x, int y);
	virtual void OnMousePress() override;

	void SetCallback(ButtonCallbackFn_t callback);

	ButtonCallbackFn_t m_pCallback;
};

#endif /* CBASEBUTTON_H_ */
