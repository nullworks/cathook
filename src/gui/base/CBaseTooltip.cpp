
/*
 *
 *  An easy way to display information to the user.
 *  Make sure to set max_size to have clamping
 *
 */

#include "CBaseTooltip.hpp"

namespace gui { namespace base {

CBaseTooltip::CBaseTooltip(const char* _name) : CBaseWidget(_name) {
  position_mode = ABSOLUTE;
}

CBaseTooltip::Draw() {

}

}}
