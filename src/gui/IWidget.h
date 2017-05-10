/*
 * IWidget.h
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#ifndef IWIDGET_H_
#define IWIDGET_H_

#include "../beforecheaders.h"
#include <utility>
#include <cstring>
#include <functional>
#include "../aftercheaders.h"

#include "../inputsystem/ButtonCode.h"

enum PositionMode {
	ABSOLUTE = 0,
	INLINE,
	FLOATING
};

class KeyValues;

class IWidget {
public:
	virtual ~IWidget();

	virtual void Update() = 0;
	virtual void Draw(int x, int y) = 0;
	virtual void DrawBounds(int x, int y) = 0;

	virtual KeyValues* Props() = 0;

	virtual void OnMouseEnter() = 0;
	virtual void OnMouseLeave() = 0;
	virtual void OnMousePress() = 0;
	virtual void OnMouseRelease() = 0;
	virtual void OnKeyPress(ButtonCode_t key, bool repeat) = 0;
	virtual void OnKeyRelease(ButtonCode_t key) = 0;
	virtual void OnFocusGain() = 0;
	virtual void OnFocusLose() = 0;

	virtual void HandleCustomEvent(KeyValues* event) = 0;

	virtual bool ConsumesKey(ButtonCode_t key) = 0;

	// Widget will be visible even when gui is turned off
	virtual bool AlwaysVisible() = 0;
	virtual void Show() = 0;
	virtual void Hide() = 0;
	virtual bool IsVisible() = 0;

	virtual bool IsHovered() = 0;
	virtual bool IsFocused() = 0;
	virtual bool IsPressed() = 0;

	virtual bool DoesStealFocus() = 0;

	virtual void SetOffset(int x, int y) = 0;
	virtual void SetMaxSize(int x, int y) = 0;
	virtual void SetSize(int x, int y) = 0;
	virtual std::pair<int, int> GetOffset() = 0;
	virtual std::pair<int, int> GetSize() = 0;
	virtual std::pair<int, int> GetMaxSize() = 0;
	virtual int GetZIndex() = 0;
	virtual void SetZIndex(int idx) = 0;

	virtual std::string GetTooltip() = 0;

	virtual PositionMode GetPositionMode() = 0;
	virtual void SetPositionMode(PositionMode) = 0;

	virtual IWidget* GetParent() = 0;
	virtual void SetParent(IWidget*) = 0;
	virtual std::string GetName() = 0;
};


#endif /* IWIDGET_H_ */
