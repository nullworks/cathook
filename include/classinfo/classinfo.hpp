/*
 * classinfo.hpp
 *
 *  Created on: May 13, 2017
 *      Author: nullifiedcat
 */

#ifndef CLASSINFO_HPP_
#define CLASSINFO_HPP_

#include "common.hpp"

#include "dummy.gen.hpp"

#include "tf2.gen.hpp"
#include "tf2c.gen.hpp"
#include "hl2dm.gen.hpp"
#include "css.gen.hpp"
#include "dynamic.gen.hpp"

#include "tf2_constexpr.gen.hpp"
#include "tf2c_constexpr.gen.hpp"
#include "hl2dm_constexpr.gen.hpp"
#include "css_constexpr.gen.hpp"

void InitClassTable();

extern client_classes::dummy *client_class_list;

#if not defined(BUILD_GAME) or defined(DYNAMIC_CLASSES)
#define CL_CLASS(x) (client_class_list->x)
#else
#define CL_CLASS(x) (client_classes_constexpr::BUILD_GAME::x)
#endif

#define RCC_CLASS(tf, hl2dm, css, def)                                         \
    (IsTF() ? CL_CLASS(tf)                                                     \
            : (IsHL2DM() ? CL_CLASS(hl2dm) : (IsCSS() ? CL_CLASS(css) : 0)))

#define RCC_PLAYER RCC_CLASS(CTFPlayer, CHL2MP_Player, CCSPlayer, 0)
#define RCC_PLAYERRESOURCE                                                     \
    RCC_CLASS(CTFPlayerResource, CPlayerResource, CCSPlayerResource, 0)

#endif /* CLASSINFO_HPP_ */
