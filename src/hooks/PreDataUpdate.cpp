/*
 * PreDataUpdate.cpp
 *
 *  Created on: Jul 27, 2018
 *      Author: bencat07
 */
#include "HookedMethods.hpp"
#include "SeedPrediction.hpp"

namespace hooked_methods
{
DEFINE_HOOKED_METHOD(PreDataUpdate, void, void *_this, int ok)
{
    hacks::tf2::seedprediction::handleFireBullets((C_TEFireBullets *) _this);
    original::PreDataUpdate(_this, ok);
}
} // namespace hooked_methods
