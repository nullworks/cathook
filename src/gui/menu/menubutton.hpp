#pragma once
#include "../base/CBaseWidget.hpp"
#include <vector>

namespace gui { namespace menu {
class CMenuButton : public base::CBaseWidget {
public:
  CMenuButton(const char *);
  virtual bool TryFocusGain();
  virtual void OnFocusLose();
  virtual void OnKeyPress(int);
  virtual void OnKeyRelease(int);
  virtual bool ConsumesKey(int);
  virtual void Hide();
  //virtual void OnMousePress();
  virtual void Draw();
  virtual void Update();

  void SetChild(IWidget *);
  //Only really relevant to keyboard input.
  bool child_focused=false;
  IWidget* child=nullptr;
  std::pair<int,int> padding=std::make_pair(3,3);
};
}}