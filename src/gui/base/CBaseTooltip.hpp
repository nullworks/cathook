
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
public:
  CBaseTooltip(const char* _name);
  virtual void Draw();
};

}}
