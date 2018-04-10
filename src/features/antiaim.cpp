
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
#include "../framework/trace.hpp"
#include "../framework/input.hpp"

#include "antiaim.hpp"

namespace features::antiaim {

static CatEnum antiaim_menu({"Anti-Aim"});
static CatVarBool enabled(antiaim_menu, "aa", false, "Enable Anti-Aim", "Master Anti-Aim switch");
static CatEnum aa_pitch_enum({"OFF", "FIXED", "UP", "DOWN", "FAKEUP", "FAKEDOWN", "FAKECENTER", "RANDOM", "BIGRANDOM", "FLIP", "FAKEFLIP"});
static CatVarEnum aa_pitch(antiaim_menu, aa_pitch_enum, "aa_pitch", 0, "AA Pitch Type", "Type of anti-aim to use for pitch");
static CatEnum aa_yaw_enum({"OFF", "FIXED", "FIXEDOFFSET", "RANDOM", "BIGRANDOM", "FLIP", "SPIN", "EDGE"});
static CatVarEnum aa_yaw(antiaim_menu, aa_yaw_enum, "aa_yaw", 0, "AA Yaw Type", "Type of anti-aim to use for yaw");
static CatVarFloat aa_pitch_amt(antiaim_menu, "aa_pitch_fixed", 0, "AA Pitch Fixed", "Value for fixed anti-aim");
static CatVarFloat aa_yaw_amt(antiaim_menu, "aa_yaw_fixed", 0, "AA Yaw Fixed", "Value for fixed anti-aim");
static CatVarFloat aa_spin_amt(antiaim_menu, "aa_spin_amt", 2.5f, "AA Spin Speed", "How fast to spin to win");
static CatVarBool allow_unclamped(antiaim_menu, "aa_unclamp", false, "AA UnClamped", "Allow UnClamped angles?\nThis is a very dangerous thing to enable if an anti-cheat is present!");
// TODO, method to change way of doing the antiaims if one would be prefered over the other depending on game
// Example: aa in csgo wouldnt work with overflowing angles, so we would use sendpacket
// othergames might not have aa but some games do, gotta make it *modular*
//static CatEnum aa_method_enum({"OVERFLOW", "PACKET-SWITCH"});
//static CatVarEnum aa_method(antiaim_menu, aa_method, "antiaim_yaw", 0, "AA Yaw Type", "Type of anti-aim to use for yaw");

// Change this to something more accurate in your game module
CMFunction<bool()> ShouldAA {[](){return !input::GetKey(CATKEY_MOUSE_1);}};

int AASafeTicks = 0;

static void WorldTick() {
  if (!enabled) return;

  if (AASafeTicks) {
    AASafeTicks--;
    return;
  }

  // Get our local ents angles
  auto local_ent = GetLocalPlayer();
  if (!local_ent || GetDormant((CatEntity*)local_ent) || !GetAlive((CatEntity*)local_ent)) return;
  auto angles = GetCameraAngle(local_ent);

  // Check if can aa
  if (!ShouldAA()) return;

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
  case 7: { // EDGE
    // TODO, fix the psudo code!
    // With edge, we need to trace until we hit walls basicly
    // Here we use a lambda to make getting the result angle easy!
    enum { EDGE_NONE, EDGE_LEFT, EDGE_RIGHT };
    auto edge_angle = []() -> auto {
      // Get our camera info
      auto local_ent = GetLocalPlayer();
      if (!local_ent || GetDormant((CatEntity*)local_ent) || !GetAlive((CatEntity*)local_ent))
        return EDGE_NONE;
      auto camera_ang = GetCameraAngle(local_ent);
      auto camera_pos = GetCamera(local_ent);
      // Get some angles facing different directions
      auto left = CatVector(0, camera_ang.y + 90);
      auto right = CatVector(0, camera_ang.y - 90);
      util::ClampAngles(left);
      util::ClampAngles(right);
      // First we go out to the left and right
      left = trace::trace_terrain(camera_pos, util::ExtendLine(camera_pos, left, 128));
      right = trace::trace_terrain(camera_pos, util::ExtendLine(camera_pos, right, 128));
      // Check if we hit a wall, if we have, return closer
      auto distto_left = camera_pos.DistTo(left);
      auto distto_right = camera_pos.DistTo(right);
      if (std::abs(distto_left - distto_right) > 2)
        return (distto_left > distto_right) ? EDGE_RIGHT : EDGE_LEFT;
      // Now we go forward and check if we hit something there
      distto_left = camera_pos.DistTo(trace::trace_terrain(camera_pos, util::ExtendLine(left, CatVector(0, camera_ang.y), 256)));
      distto_right = camera_pos.DistTo(trace::trace_terrain(camera_pos, util::ExtendLine(right, CatVector(0, camera_ang.y), 256)));
      if (std::abs(distto_left - distto_right) > 4)
        return (distto_left > distto_right) ? EDGE_RIGHT : EDGE_LEFT;
      // If we havent hit anything, then we return none
      return EDGE_NONE;
    }();
    if (edge_angle != EDGE_NONE) {
      // Pitch can affect how our head is behind walls, so we fix that here
      // we also clamp so we edge where our real head is
      float pitch = angles.x;
      while (pitch > 89) // Clamping :joy:
        pitch -= 180;
      while (pitch < -89)
        pitch += 180;
      if (pitch < 0) {
        if (edge_angle == EDGE_LEFT)
          edge_angle = EDGE_RIGHT;
        else
          edge_angle = EDGE_LEFT;
      }
      // Do the edge dance
      if (edge_angle == EDGE_LEFT)
        angles.y += 90;
      else
        angles.y -= 90;
    }
  }
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

}
