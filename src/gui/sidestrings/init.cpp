
/*
 *
 *	This is an init file for hudstrings!
 *
 */

#include "../../framework/drawmgr.hpp"
#include "sidestrings.hpp"

namespace gui { namespace sidestrings {

void Init() {
	drawmgr::RequestDrawOnBefore(DefaultSideStrings);
	drawmgr::RequestDrawOnAfter(DrawSideStrings);
	BuildTopString();
}
	
}}