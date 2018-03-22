#pragma once
#include "../base/CBaseParent.hpp"
#include <vector>

namespace gui { namespace menu {
class CMenuButton : public base::CBaseParent {
public:
  CMenuButton(std::string name);
  virtual bool TryFocusGain();
  virtual void OnFocusLose();
  virtual void OnKeyPress(int, bool);
  virtual bool ConsumesKey(int);
  //virtual void OnMousePress();
  virtual void Draw();
  virtual void UpdatePositioning();
	void AddChild(base::CBaseWidget* child);

  //Only really relevant to keyboard input.
  std::pair<int,int> padding=std::make_pair(3,3);
};
}}