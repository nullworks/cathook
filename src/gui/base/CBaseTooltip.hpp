
/*
 *
 *  A nice box that displays info.
 *  Set the max size and if the text given to it is too big, the box will wrap the text downwards.
 *
 *
 */

#pragma once

#include "CBaseWidget.hpp"

namespace gui { namespace base {

class CBaseTooltip : public CBaseWidget {
protected:
  std::pair<int,int> max_size;
public:
  CBaseTooltip(std::string name,std::pair<int,int> max_size = std::make_pair(-1,-1));
  virtual void Draw();
};

}}
