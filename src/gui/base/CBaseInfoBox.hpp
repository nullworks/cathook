
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

class CBaseInfoBox : public CBaseWidget {
public:
  CBaseInfoBox(std::string _name, std::string info = "unset", std::pair<int, int> max_size = std::make_pair(-1, -1));
  virtual ~CBaseInfoBox(){}
  virtual void Draw();
  std::string infostring;
  std::pair<int, int> max_size;
};

}}
