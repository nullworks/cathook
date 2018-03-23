
/*
 *
 * bootiful shoddy bhop
 *
 */

#include "../util/logging.hpp"
#include "../util/catvars.hpp"
#include "../framework/gameticks.hpp"
#include "../framework/prediction.hpp"

#include "bhop.hpp"

namespace features::bhop {

static CatEnum bhop_menu({"Misc"});
static CatVarBool bhop(bhop_menu, "bhop", false, "Bunny Hop");
static CatVarKey bhop_key(bhop_menu, "bhop_key", CATKEY_SPACE, "Key used for bunnyhop");
static CatVarInt bhop_ticks(bhop_menu, "bhop_ticks", 5, "Ticks to wait before attempting to jump");

// The main problem here is that source likes to not jump while holding jump key...
// I dont know how to modularize removing jump without adding a remove jump to framework/entitys :/
// TODO: Finish this to get it working...

static void WorldTick(){
  if (!bhop) return;

  auto local_ent = GetLocalPlayer();
  if (!local_ent || GetDormant((CatEntity*)local_ent) || !GetAlive((CatEntity*)local_ent))
    return;

  static int jump_ticks = 0;
  if (bhop_key.Depressed() && OnGround((CatEntity*)local_ent) && jump_ticks++ >= bhop_ticks) {
    Jump(local_ent);
    return;
  }
  jump_ticks = 0;
}

void Init(){
  wtickmgr.REventDuring(WorldTick);
}

}
