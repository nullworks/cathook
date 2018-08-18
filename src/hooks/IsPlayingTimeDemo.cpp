/*
 * IsPlayingTimeDemo.cpp
 *
 *  Created on: May 13, 2018
 *      Author: bencat07
 */
#include "HookedMethods.hpp"
#include "MiscTemporary.hpp"

namespace hooked_methods
{
DEFINE_HOOKED_METHOD(IsPlayingTimeDemo, bool)
{
    if (nolerp)
        return true;
    else
        return original::IsPlayingTimeDemo();
}
} // namespace hooked_methods
