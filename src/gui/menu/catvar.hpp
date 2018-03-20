#pragma once
#include "../base/CBaseWidget.hpp"
#include "../../util/catvars.hpp"
#include <vector>

namespace gui { namespace menu {
class CCatVar : public base::CBaseWidget {
public:
  CCatVar(CatVar *);
  virtual void OnFocusLose();
  virtual void OnKeyPress(int, bool);
  virtual void OnKeyRelease(int);
  virtual bool ConsumesKey(int);
  virtual void Draw();
  virtual void UpdatePositioning();

  int min_mid_padding=3;
  std::pair<int,int> padding=std::make_pair(3,3);
  std::string content;
  bool typing=false;
  int cursor;
	std::chrono::steady_clock::time_point blink_start_time;
  CatVar * cv;
  //TODO: CatVar to configure blink?
  const float blink_period=1;
  const float blink_vis_time=0.6f;
  
};
}}