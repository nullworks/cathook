/*
 * bhop.hpp
 *
 *  Created on: Jun 5, 2017
 *      Author: nullifiedcat
 */

#pragma once

class KeyValues;
class CachedEntity;

namespace ac
{
namespace bhop
{

void ResetEverything();
void ResetPlayer(int idx);

void Init();
void Update(CachedEntity *player);
void Event(KeyValues *event);
}
}
