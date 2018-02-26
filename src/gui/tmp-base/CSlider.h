/*
 * CSlider.h
 *
 *  Created on: Jan 27, 2017
 *      Author: nullifiedcat
 */

#ifndef CSLIDER_H_
#define CSLIDER_H_

#include "CBaseWidget.h"

class CSlider;

typedef std::function<void(CSlider*, float, float)> SliderCallbackFn_t;

class CSlider : public CBaseWidget {
public:
	CSlider(std::string name = "", IWidget* parent = nullptr);

	void Setup(float min, float max);
	void SetStep(float step);
	float Value();
	void SetValue(float value);
	void SetCallback(SliderCallbackFn_t callback);

	virtual void Update();
	virtual void Draw(int x, int y);

	SliderCallbackFn_t m_pCallback;

	int m_nLastX;
	bool m_bDragInit;
	int m_nSliderPos;

};


#endif /* CSLIDER_H_ */
