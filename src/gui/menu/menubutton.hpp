#pragma once
#include "../base/CBaseWidget.hpp"
#include <vector>

namespace gui { namespace menu {
class CMenuButton : public base::CBaseWidget {
public:
  CMenuButton(std::string name);
  virtual bool TryFocusGain();
  virtual void OnFocusLose();
  virtual void OnKeyPress(int, bool);
  virtual void OnKeyRelease(int);
  virtual bool ConsumesKey(int);
  //virtual void OnMousePress();
  virtual void Draw();
  virtual void UpdatePositioning();

  void SetChild(IWidget *);
  //Only really relevant to keyboard input.
  bool child_focused=false;
  IWidget* child=nullptr;
  std::pair<int,int> padding=std::make_pair(3,3);
};
}}