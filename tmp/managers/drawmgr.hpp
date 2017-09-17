/*
 * drawmgr.hpp
 *
 *  Created on: May 22, 2017
 *      Author: nullifiedcat
 */

#ifndef DRAWMGR_HPP_
#define DRAWMGR_HPP_

#include "beforecheaders.h"
#include <mutex>
#include "aftercheaders.h"

extern std::mutex drawing_mutex;

void BeginCheatVisuals();
void DrawCheatVisuals();
void EndCheatVisuals();

#endif /* DRAWMGR_HPP_ */
