/*
 * CBaseWidget.h
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#ifndef CBASEWIDGET_H_
#define CBASEWIDGET_H_

#include "IWidget.h"
#include <memory>
#include <vector>
#include <cstring>
#include <algorithm>

#include "../../util/logging.hpp"

#include "KeyValues.h"

class CBaseWidget : public virtual IWidget {
public:
	inline ~CBaseWidget() {}
	CBaseWidget(std::string name = "unnamed", IWidget* parent = nullptr);
	Color bounds_color;
	virtual void Update();
	inline virtual void Draw(int x, int y) {};
	virtual void DrawBounds(int x, int y);

	inline virtual KeyValues* Props() {
		return &m_KeyValues;
	}

	inline virtual void OnMouseEnter() { m_KeyValues.SetBool("hover", true); }
	inline virtual void OnMouseLeave() { m_KeyValues.SetBool("hover", false); }
	inline virtual void OnMousePress() { m_KeyValues.SetBool("press", true); }
	inline virtual void OnMouseRelease() { m_KeyValues.SetBool("press", false); }
	inline virtual void OnKeyPress(int key, bool repeat) {};
	inline virtual void OnKeyRelease(int key) {};
	inline virtual void OnFocusGain() { m_KeyValues.SetBool("focus", true); }
	inline virtual void OnFocusLose() {	m_KeyValues.SetBool("focus", false); }

	inline virtual void HandleCustomEvent(KeyValues* event) {};

	inline virtual bool ConsumesKey(int key) { return false; }

	inline virtual bool AlwaysVisible() { return m_KeyValues.GetBool("always_visible"); }

	inline virtual void Show() { m_KeyValues.SetBool("visible", true); }
	inline virtual void Hide() { m_KeyValues.SetBool("visible", false); }
	inline virtual bool IsVisible() {
		if (GetParent()) return GetParent()->IsVisible() && m_KeyValues.GetBool("visible");
		return m_KeyValues.GetBool("visible");
	}

	virtual bool IsHovered();
	virtual bool IsFocused();
	virtual bool IsPressed();

	inline virtual bool DoesStealFocus() { return true; }

	inline virtual void SetOffset(int x, int y) {
		if (x >= 0) m_KeyValues.SetInt("offset_x", x);
		if (y >= 0) m_KeyValues.SetInt("offset_y", y);
	}
	inline virtual void SetMaxSize(int x, int y) {
		if (x >= 0) m_KeyValues.SetInt("max_x", x);
		if (y >= 0) m_KeyValues.SetInt("max_y", y);
	}
	inline virtual std::pair<int, int> GetOffset() {
		return std::make_pair(m_KeyValues.GetInt("offset_x"), m_KeyValues.GetInt("offset_y"));
	}
	inline virtual std::pair<int, int> GetSize() {
		return std::make_pair(m_KeyValues.GetInt("size_x"), m_KeyValues.GetInt("size_y"));
	}
	inline virtual std::pair<int, int> GetMaxSize() {
		return std::make_pair(m_KeyValues.GetInt("max_x"), m_KeyValues.GetInt("max_y"));
	}
	inline virtual int GetZIndex() { return m_KeyValues.GetInt("zindex"); }
	inline virtual void SetZIndex(int idx) { m_KeyValues.SetInt("zindex", idx); }

	inline virtual std::string GetTooltip() { return std::string(m_KeyValues.GetString("tooltip")); }

	inline virtual PositionMode GetPositionMode() { return (PositionMode)m_KeyValues.GetInt("positionmode"); }
	inline virtual void SetPositionMode(PositionMode mode) { m_KeyValues.SetInt("positionmode", mode); };

	inline virtual IWidget* GetParent() { return m_pParent; }
	inline virtual void SetParent(IWidget* parent) { m_pParent = parent; }
	inline virtual std::string GetName() { return std::string(m_KeyValues.GetString("name")); }

	std::pair<int, int> AbsolutePosition();
	inline void SetSize(int x, int y) {
		if (x >= 0) m_KeyValues.SetInt("size_x", x);
		if (y >= 0) m_KeyValues.SetInt("size_y", y);
	}

	KeyValues m_KeyValues;
	IWidget* m_pParent;

};


#endif /* CBASEWIDGET_H_ */
