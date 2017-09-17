/*
 * Radar.hpp
 *
 *  Created on: Mar 28, 2017
 *      Author: nullifiedcat
 */

#ifndef HACKS_RADAR_HPP_
#define HACKS_RADAR_HPP_

#include "../common.h"

namespace hacks { namespace tf { namespace radar {

extern std::unique_ptr<textures::AtlasTexture> tx_classes[3][9];
extern std::unique_ptr<textures::AtlasTexture> tx_teams[2];
extern std::unique_ptr<textures::AtlasTexture> tx_items[2];

extern CatVar size;
extern CatVar zoom;
extern CatVar radar_enabled;
extern CatVar radar_x;
extern CatVar radar_y;


void Init();
std::pair<int, int> WorldToRadar(int x, int y);
void Draw();

}}}

#endif /* HACKS_RADAR_HPP_ */
