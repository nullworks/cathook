/*
 * drawmgr.hpp
 *
 *  Created on: May 22, 2017
 *      Author: nullifiedcat
 */

#ifndef DRAWMGR_HPP_
#define DRAWMGR_HPP_

#include "beforecheaders.hpp"
#include <mutex>
#include "aftercheaders.hpp"

extern std::mutex drawing_mutex;

void render_cheat_visuals();

void BeginCheatVisuals();
void DrawCheatVisuals();
void EndCheatVisuals();

#endif /* DRAWMGR_HPP_ */
