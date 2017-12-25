
/*
 *
 *  An easy way to display information to the user.
 *  Make sure to set max_size to have clamping
 *
 */

#include "CBaseInfoBox.hpp"

namespace gui { namespace base {

CBaseInfoBox::CBaseInfoBox(const char* _name) : CBaseWidget(_name) {
  position_mode = ABSOLUTE;
}

CBaseInfoBox::Draw() {
  
}

}}
