
/*
 *
 * bootiful shoddy bhop
 *
 */

#include "../util/catvars.hpp"
#include "../framework/gameticks.hpp"
#include "../framework/prediction.hpp"

#include "bhop.hpp"

namespace features::bhop {

static CatEnum bhop_menu({"Misc"});
static CatVarBool bhop(bhop_menu, "bhop", false, "Bunny Hop");

static void WorldTick(){
  if (!bhop) return;

  auto local_ent = GetLocalPlayer();
  if (!local_ent || GetDormant((CatEntity*)local_ent) || !GetAlive((CatEntity*)local_ent))
    return;

  if (!OnGround((CatEntity*)local_ent))
    return;
}

void Init(){
  wtickmgr.REventDuring(WorldTick);
}

}
