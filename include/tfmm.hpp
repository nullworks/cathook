/*
 * tfmm.hpp
 *
 *  Created on: Dec 7, 2017
 *      Author: nullifiedcat
 */

#pragma once

namespace tfmm
{

void queue_start();
void queue_leave();
void dcandabandon();
void abandon();
extern CatVar queue;
extern CatEnum queue_mode;
}
