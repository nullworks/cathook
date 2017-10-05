 
/*
 *
 *	The header file for world to 
 *
 */

#include "../util/mathlib.hpp"

namespace wts {

bool WorldToScreen(CatVector& world, CatVector& screen);
void Refresh();
	
void InitWorldToScreen(bool *func(CatVector&, CatVector&));
//void InitRefresh(void *func());

}