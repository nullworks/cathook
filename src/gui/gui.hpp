
/*
 * GUI.h
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "base/CBaseContainer.hpp"
#include "../util/catvars.hpp"

namespace gui {

extern base::CBaseContainer g_pGui;
extern CatVarKey activatekey;
extern CatVarKey backkey;
extern CatVarKey nextkey;
extern CatVarKey prevkey;
extern CatVarKey field_leftkey;
extern CatVarKey field_rightkey;
extern CatVarKey field_escapekey;

}
