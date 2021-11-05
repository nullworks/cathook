/*
 * classinfo.hpp
 *
 *  Created on: May 13, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "config.h"
#include "dummy.gen.hpp"
#include "tf2.gen.hpp"
#include "tf2_constexpr.gen.hpp"

void InitClassTable();
extern client_classes::dummy *client_class_list;

#define CL_CLASS(x) (client_classes_constexpr::tf2::x)
#define RCC_CLASS(tf) CL_CLASS(tf)
#define RCC_PLAYER RCC_CLASS(CTFPlayer)
#define RCC_PLAYERRESOURCE RCC_CLASS(CTFPlayerResource)
