/*
 * CatBot.hpp
 *
 *  Created on: Dec 30, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "common.hpp"

namespace hacks { namespace shared { namespace catbot {

bool is_a_catbot(unsigned steamID);
bool should_ignore_player(CachedEntity *player);
void update_ipc_data(ipc::user_data_s& data);
void update();
void init();

}
}
}
