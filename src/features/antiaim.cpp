
/*
 *
 * Nekohooks shoddy antiaim!
 * This absolutly requires a set silent angle function... But maybe not after I impliment another way of silent :3
 * TODO, set camera angle to the anti aim angle, then occasionally snapback to old angle, screenshot and display that below the menu and everything...
 * So the antiaim does its magic and you just get a laggy screen... Great!
 * NOTE: Some of the aas were ripped from cathook, partial credit goes to nullified cat
 *
 */

#include "../util/catvars.hpp"
#include "../framework/gameticks.hpp"

#include "antiaim.hpp"

namespace features { namespace antiaim {

static CatEnum antiaim_menu({"Anti-Aim"});
static CatVarBool enabled(antiaim_menu, "antiaim", false, "Enable Anti-Aim", "Master Anti-Aim switch");
static CatVarEnum aa_pitch(antiaim_menu, {"OFF", "FIXED", "UP", "DOWN", "FAKEUP", "FAKEDOWN", "FAKECENTER", "RANDOM", "BIGRANDOM", "FLIP", "FAKEFLIP"}, "antiaim_pitch", 0, "AA Pitch Type", "Type of anti-aim to use for pitch");
static CatVarEnum aa_yaw(antiaim_menu, {"OFF", "FIXED", "FIXEDOFFSET", "RANDOM", "BIGRANDOM", "FLIP", "SPIN", "EDGE"}, "antiaim_yaw", 0, "AA Yaw Type", "Type of anti-aim to use for yaw");
static CatVarFloat aa_pitch_amt(antiaim_menu, "antiaim_pitch_fixed", 0, "AA Pitch Fixed", "Value for fixed anti-aim");
static CatVarFloat aa_yaw_amt(antiaim_menu, "antiaim_yaw_fixed", 0, "AA Yaw Fixed", "Value for fixed anti-aim");
static CatVarFloat aa_spin_amt(antiaim_menu, "antiaim_spin_amt", 2.5f, "AA Spin Speed", "How fast to spin to win");
static CatVarBool allow_unclamped(antiaim_menu, "antiaim_unclamp", false, "AA UnClamped", "Allow UnClamped angles?\nThis is a very dangerous thing to enable if an anti-cheat is present!");

static void WorldTick() {
  if (!enabled) return;

  // Get our local ents angles
  auto local_ent = GetLocalPlayer();
  if (!local_ent || GetDormant((CatEntity*)local_ent) || !GetAlive((CatEntity*)local_ent)) return;
  auto angles = GetCameraAngle(local_ent);

  // Pitch
  switch(aa_pitch) {
  case 1: // FIXED
    angles.x = aa_pitch_amt;
    break;
  case 2: // UP
    angles.x = -89.0f;
    break;
  case 3: // Down
    angles.x = 89.0f;
    break;
  case 4: // CENTER
    angles.x = 0.0f;
    break;
  case 5: // FAKEUP
    angles.x = -271.0f;
    break;
  case 6: // FAKEDOWN
    angles.x = 271.0f;
    break;
  case 7: // FAKECENTER
    angles.x = -3256.0f;
    break;
  case 8: // RANDOM
    angles.x = util::RandFloatRange(-180.0f, 180.0f);
    break;
  case 9: // BIGRANDOM
    angles.x = util::RandFloatRange(-65536.0f, 65536.0f);
    break;
  case 10: // FLIP
    static bool flip;
    angles.x = flip ? -89 : 89;
    flip = !flip;
    break;
  case 11: // FAKEFLIP
    angles.x = flip ? -271.0f : 271.0f;
    flip = !flip;
    break;
  }

  // Yaw
  switch(aa_yaw) {
  case 1: // FIXED
    angles.y = aa_yaw_amt;
    break;
  case 2: // FIXEDOFFSET
    angles.y += aa_yaw_amt;
    break;
  case 3: // RANDOM
    angles.y = util::RandFloatRange(-180.0f, 180.0f);
    break;
  case 4: // BIGRANDOM
    angles.y = util::RandFloatRange(-65536.0f, 65536.0f);
    break;
  case 5: // FLIP
    static bool flip;
    angles.y += flip ? -90 : 90;
    flip = !flip;
    break;
  case 6: // SPIN
    static float cur_yaw;
    cur_yaw += aa_spin_amt;
    while (cur_yaw > 180) // Clamping :joy:
      cur_yaw -= 360;
    while (cur_yaw < -180)
      cur_yaw += 360;
    angles.y = cur_yaw;
    break;
  //case 7: // EDGE
    // TODO BOIIIIISSSS
  }

  // Clamp Finished angles
  if (!allow_unclamped)
    util::ClampAngles(angles);

  // Send it off to the races
  SetSilentCameraAngle(local_ent, angles);
}

// TODO, snapback/screenshot antiaim
/*static void DrawTick() {

}*/

void Init() {

  wtickmgr.REventDuring(WorldTick);

}

}}
