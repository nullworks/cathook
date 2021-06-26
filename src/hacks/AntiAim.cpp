/*
 * AntiAim.cpp
 *
 *  Created on: Oct 26, 2016
 *      Author: nullifiedcat
 */

#include <hacks/hacklist.hpp>
#include <settings/Bool.hpp>
#include <hacks/AntiAim.hpp>

#include "common.hpp"
namespace hacks::shared::antiaim
{
bool force_fakelag = false;
float used_yaw     = 0.0f;
static settings::Boolean enable{ "antiaim.enable", "0" };

static settings::Boolean no_clamping{ "antiaim.no-clamp", "0" };
static settings::Float roll{ "antiaim.roll", "0" };
static settings::Float spin{ "antiaim.spin-speed", "10" };

static settings::Int pitch_fake{ "antiaim.pitch.fake", "0" };
static settings::Int pitch_real{ "antiaim.pitch.real", "0" };
static settings::Float pitch_static{ "antiaim.pitch.static", "0" };

static settings::Int yaw_fake{ "antiaim.yaw.fake", "0" };
static settings::Float yaw_fake_static{ "antiaim.yaw.fake.static", "0" };
static settings::Int yaw_real{ "antiaim.yaw.real", "0" };
static settings::Float yaw_real_static{ "antiaim.yaw.real.static", "0" };

static settings::Boolean aaaa_enable{ "antiaim.aaaa.enable", "0" };
static settings::Float aaaa_interval{ "antiaim.aaaa.interval.seconds", "0" };
static settings::Float aaaa_interval_random_high{ "antiaim.aaaa.interval.random-high", "10" };
static settings::Float aaaa_interval_random_low{ "antiaim.aaaa.interval.random-low", "2" };
static settings::Int aaaa_mode{ "antiaim.aaaa.mode", "0" };
static settings::Button aaaa_flip_key{ "antiaim.aaaa.flip-key", "<null>" };

// Unused?
static settings::Int yaw_sideways_min{ "antiaim.yaw.sideways.min", "0" };
static settings::Int yaw_sideways_max{ "antiaim.yaw.sideways.max", "4" };

// Two values for fake and real angles
float cur_yaw[2] = {
	0.0f,
	0.0f
};

int safe_space = 0;

float aaaa_timer_start = 0.0f;
float aaaa_timer       = 0.0f;
int aaaa_stage         = 0;
bool aaaa_key_pressed  = false;

float GetAAAAPitch()
{
    switch ((int) aaaa_mode)
    {
    case 0:
        return aaaa_stage ? -271 : -89;
    case 1:
        return aaaa_stage ? 271 : 89;
    case 2:
        return aaaa_stage ? -180 : 180;
    default:
        break;
    }
    return 0;
}

float GetAAAATimerLength()
{
    if (aaaa_interval)
    {
        return (float) aaaa_interval;
    }
    else
    {
        return RandFloatRange((float) aaaa_interval_random_low, (float) aaaa_interval_random_high);
    }
}

void NextAAAA()
{
    aaaa_stage++;
    // TODO temporary..
    if (aaaa_stage > 1)
        aaaa_stage = 0;
}

void UpdateAAAAKey()
{
    if (aaaa_flip_key.isKeyDown())
    {
        if (!aaaa_key_pressed)
        {
            aaaa_key_pressed = true;
            NextAAAA();
        }
    }
    else
        aaaa_key_pressed = false;
}

void UpdateAAAATimer()
{
    const float &curtime = g_GlobalVars->curtime;
    if (aaaa_timer_start > curtime)
        aaaa_timer_start = 0.0f;
    if (!aaaa_timer || !aaaa_timer_start)
    {
        aaaa_timer       = GetAAAATimerLength();
        aaaa_timer_start = curtime;
    }
    else
    {
        if (curtime - aaaa_timer_start > aaaa_timer)
        {
            NextAAAA();
            aaaa_timer_start = curtime;
            aaaa_timer       = GetAAAATimerLength();
        }
    }
}

enum k_EFuckMode
{
    FM_INCREMENT,
    FM_RANDOMVARS,
    FM_JITTER,
    FM_SIGNFLIP,
    FM_COUNT
};

struct FuckData_s
{
    float fl1, fl2, fl3, fl4;
    int i1, i2;
    bool b1, b2;
};

/*
 * Not yet implemented.
 */

void FuckPitch(float &io_pitch)
{
    constexpr float min_pitch = -149489.97f;
    constexpr float max_pitch = 149489.97f;
    // static FuckData_s fuck_data;
    static k_EFuckMode fuckmode = k_EFuckMode::FM_RANDOMVARS;
    // static int fuckmode_ticks   = 0;

    /*if (!fuckmode_ticks) {
        fuckmode = rand() % k_EFuckMode::FM_COUNT;
        fuckmode_ticks = rand() % 333;
        switch (fuckmode) {
        case k_EFuckMode::FM_INCREMENT:
            fuck_data.fl1 = RandFloatRange(-400.0f, 400.0f);
            fuck_data.i1 = rand() % 3;
            break;
        case k_EFuckMode::FM_JITTER:
            fuck_data.fl1 = RandFloatRange(1.0f, 4.0f);
            break;
        case k_EFuckMode::FM_RANDOMVARS:
            break;
        }
    }*/

    switch (fuckmode)
    {
    case k_EFuckMode::FM_RANDOMVARS:
        io_pitch = RandFloatRange(min_pitch, max_pitch);
    default:
        break;
    }

    if (io_pitch < min_pitch)
        io_pitch = min_pitch;
    if (io_pitch > max_pitch)
        io_pitch = max_pitch;
}

void FuckYaw(float &io_yaw)
{
    constexpr float min_yaw = -359999.97f;
    constexpr float max_yaw = 359999.97f;

    static k_EFuckMode fuckmode = k_EFuckMode::FM_RANDOMVARS;

    switch (fuckmode)
    {
    case k_EFuckMode::FM_RANDOMVARS:
        io_yaw = RandFloatRange(min_yaw, max_yaw);
    default:
        break;
    }

    if (io_yaw < min_yaw)
        io_yaw = min_yaw;
    if (io_yaw > max_yaw)
        io_yaw = max_yaw;
}

void SetSafeSpace(int safespace)
{
    if (safespace > safe_space)
        safe_space = safespace;
}

/* checks if action slot is being used */
void SendNetMessage(INetMessage &msg)
{
    if (!enable)
        return;

    if(!((KeyValues *) (((unsigned *) &msg)[4])))
        return;

    auto name = ((KeyValues *) (((unsigned *) &msg)[4]))->GetName();

    if (CE_BAD(LOCAL_E))
        return;

    /* checks if action slot has been used & grapple is equipped */
    if (!strcmp(name, "+use_action_slot_item_server") && HasWeapon(LOCAL_E, 1152))
        SetSafeSpace(2);
}

bool ShouldAA(CUserCmd *cmd)
{
    if (hacks::tf2::antibackstab::noaa)
        return false;
    if (cmd->buttons & IN_USE)
        return false;
    int classid = LOCAL_W->m_iClassID();
    auto mode   = GetWeaponMode();
    if ((cmd->buttons & IN_ATTACK) && !(IsTF2() && (classid == CL_CLASS(CTFCompoundBow) || mode == weapon_melee)) && CanShoot())
    {
        return false;
    }
    if ((cmd->buttons & IN_ATTACK2) && classid == CL_CLASS(CTFLunchBox))
        return false;
    if ((cmd->buttons & IN_ATTACK) && classid == CL_CLASS(CTFGrapplingHook) && !g_pLocalPlayer->bAttackLastTick)
    {
        SetSafeSpace(2);
    }
    switch (mode)
    {
    case weapon_projectile:
        if (classid == CL_CLASS(CTFCompoundBow))
        {
            if (!(cmd->buttons & IN_ATTACK))
            {
                if (g_pLocalPlayer->bAttackLastTick)
                    SetSafeSpace(4);
            }
            break;
        }
        [[fallthrough]];
    case weapon_throwable:
        if ((cmd->buttons & (IN_ATTACK | IN_ATTACK2)) || g_pLocalPlayer->bAttackLastTick)
        {
            SetSafeSpace(8);
            return false;
        }
        break;
    case weapon_melee:
        if (g_pLocalPlayer->weapon_melee_damage_tick)
            return false;
        // Spy knife needs special treatment. There is no delay between IN_ATTACK and a hit
        if (g_pLocalPlayer->clazz == tf_class::tf_spy && cmd->buttons & IN_ATTACK && CanShoot())
            return false;
    default:
        break;
    }
    if (safe_space)
    {
        safe_space--;
        if (safe_space < 0)
            safe_space = 0;
        return false;
    }
    return true;
}

// Initialize Edge vars
float edgeYaw      = 0;
float edgeToEdgeOn = 0;

// Function to return distance from you to a yaw directed to
float edgeDistance(float edgeRayYaw)
{
    // Main ray tracing area
    trace_t trace;
    Ray_t ray;
    Vector forward;
    float sp, sy, cp, cy;
    sy        = sinf(DEG2RAD(edgeRayYaw)); // yaw
    cy        = cosf(DEG2RAD(edgeRayYaw));
    sp        = sinf(DEG2RAD(0)); // pitch
    cp        = cosf(DEG2RAD(0));
    forward.x = cp * cy;
    forward.y = cp * sy;
    forward.z = -sp;
    forward   = forward * 300.0f + g_pLocalPlayer->v_Eye;
    ray.Init(g_pLocalPlayer->v_Eye, forward);
    // trace::g_pFilterNoPlayer to only focus on the enviroment
    g_ITrace->TraceRay(ray, 0x4200400B, &trace::filter_no_player, &trace);
    // Pythagorean theorem to calculate distance
    float edgeDistance = (sqrt(pow(trace.startpos.x - trace.endpos.x, 2) + pow(trace.startpos.y - trace.endpos.y, 2)));
    return edgeDistance;
}

// Function to Find an edge and report if one is found at all
bool findEdge(float edgeOrigYaw)
{
    // distance two vectors and report their combined distances
    float edgeLeftDist  = edgeDistance(edgeOrigYaw - 21);
    edgeLeftDist        = edgeLeftDist + edgeDistance(edgeOrigYaw - 27);
    float edgeRightDist = edgeDistance(edgeOrigYaw + 21);
    edgeRightDist       = edgeRightDist + edgeDistance(edgeOrigYaw + 27);

    // If the distance is too far, then set the distance to max so the angle
    // isnt used
    if (edgeLeftDist >= 260)
        edgeLeftDist = 999999999;
    if (edgeRightDist >= 260)
        edgeRightDist = 999999999;

    // If none of the vectors found a wall, then dont edge
    if (edgeLeftDist == edgeRightDist)
        return false;

    // Depending on the edge, choose a direction to face
    if (edgeRightDist < edgeLeftDist)
    {
        edgeToEdgeOn = 1;
        // Correction for pitches to keep the head behind walls with real Up or Jitter
        if ((((int) pitch_real == 2) || ((int) pitch_real == 4)) && !g_pLocalPlayer->isFakeAngleCM)
            edgeToEdgeOn = 2;
        return true;
    }
    else
    {
        edgeToEdgeOn = 2;
        // Same as above
       if ((((int) pitch_real == 2) || ((int) pitch_real == 4)) && !g_pLocalPlayer->isFakeAngleCM)
            edgeToEdgeOn = 1;
        return true;
    }
}

// Function to give you a static angle to use
float useEdge(float edgeViewAngle)
{
    // Var to be disabled when a angle is choosen to prevent the others from
    // conflicting
    bool edgeTest = true;
    if (((edgeViewAngle < -135) || (edgeViewAngle > 135)) && edgeTest == true)
    {
        if (edgeToEdgeOn == 1)
            edgeYaw = (float) -90;
        if (edgeToEdgeOn == 2)
            edgeYaw = (float) 90;
        edgeTest = false;
    }
    if ((edgeViewAngle >= -135) && (edgeViewAngle < -45) && edgeTest == true)
    {
        if (edgeToEdgeOn == 1)
            edgeYaw = (float) 0;
        if (edgeToEdgeOn == 2)
            edgeYaw = (float) 179;
        edgeTest = false;
    }
    if ((edgeViewAngle >= -45) && (edgeViewAngle < 45) && edgeTest == true)
    {
        if (edgeToEdgeOn == 1)
            edgeYaw = (float) 90;
        if (edgeToEdgeOn == 2)
            edgeYaw = (float) -90;
        edgeTest = false;
    }
    if ((edgeViewAngle <= 135) && (edgeViewAngle >= 45) && edgeTest == true)
    {
        if (edgeToEdgeOn == 1)
            edgeYaw = (float) 179;
        if (edgeToEdgeOn == 2)
            edgeYaw = (float) 0;
        edgeTest = false;
    }
    // return with the angle choosen
    return edgeYaw;
}
static float randyaw = 0.0f;
void ProcessUserCmd(CUserCmd *cmd)
{
	// Not running
    if (!enable)
        return;
    if (!ShouldAA(cmd))
        return;
    if (!pitch_fake && !pitch_real && !yaw_fake && !yaw_real)
        return;
    
    static bool keepmode = true;
    keepmode             = !keepmode;
    float &p             = cmd->viewangles.x;
    float &y             = cmd->viewangles.y;
    static bool flip     = false;
    bool clamp           = !no_clamping;
    bool yaw_mode		 = true;

    static int ticksUntilSwap = 0;
    static bool swap          = true;
	
	// Reset the ticks and swap for some reason...
    if (ticksUntilSwap > 0 && (*yaw_fake != 8 || *yaw_real != 8))
    {
        swap           = true;
        ticksUntilSwap = 0;
    }
    
    // Yaw logic
    if (g_pLocalPlayer->isFakeAngleCM)
		yaw_mode = false;
	
	switch ((int) (yaw_mode ? yaw_real : yaw_fake))
	{
	case 1: // Custom
		y = (float) (yaw_mode ? yaw_real_static : yaw_fake_static);
		break;
	case 2: // Custom Offset
		y += (float) (yaw_mode ? yaw_real_static : yaw_fake_static);
		break;
	case 3: // Left
		y -= 90.0f;
		break;
	case 4: // Right
		y += 90.0f;
		break;
	case 5: // Back
		y += 180.0f;
		break;
	case 6: // Spin
		cur_yaw[yaw_mode] += yaw_mode ? (float) spin : -((float) spin);
		while (cur_yaw[yaw_mode] > 180.0f)
			cur_yaw[yaw_mode] += -360.0f;
		while (cur_yaw[yaw_mode] < -180.0f)
			cur_yaw[yaw_mode] += 360.0f;
		y = cur_yaw[yaw_mode];
		break;
	case 7: // Edge
		// Attempt to find an edge and if found, rotate around it
		if (findEdge(y))
			y = useEdge(y);
		break;
	case 8: // Sideways
		if (!yaw_mode)
			swap = !swap;
		y += swap ? 90.0f : -90.0f;
		break;
	case 9: // Heck
		FuckYaw(y);
		clamp = false;
		break;
	case 10: // Omega
		if (!yaw_mode)
        {
            randyaw += RandFloatRange(-30.0f, 30.0f);
            y = randyaw;
        }
        else
            y = randyaw - 180.0f + RandFloatRange(-40.0f, 40.0f);
		break;
	case 11: // Random
		y     = RandFloatRange(-65536.0f, 65536.0f);
		clamp = false;
		break;
	case 12: // Random Clamped
		y = RandFloatRange(-180.0f, 180.0f);
		break;
	default:
		break;
	}
    
    // Pitch logic
    switch (int(pitch_real))
    {
    case 1: // Custom
        p = float(pitch_static);
        break;
    case 2: // Up
        p = -89.0f;
        break;
    case 3: // Down
        p = 89.0f;
        break;
    case 4: // Jitter
        if (flip)
            p += 30.0f;
        else
            p -= 30.0f;
        break;
    case 5: // Random
        p = RandFloatRange(-89.0f, 89.0f);
        break;
    case 6: // Flip
        p = flip ? 89.0f : -89.0f;
        break;
    case 7: // Heck
        FuckPitch(p);
        clamp = false;
    }
    
    // Fake is done afterwards so that they can be applied on top of the real angles set above
    switch (int(pitch_fake))
    {
    case 1: // Up
        p -= 360.0f;
        break;
    case 2: // Down
        p += 360.0f;
        break;
    case 3: // Inverse
		if (p <= -89.0f)
			p += 360.0f;
		else if (p >= 89.0f)
			p -= 360.0f;
        break;
    }
    
    flip = !flip;
    if (clamp)
        fClampAngle(cmd->viewangles);
    if (roll)
        cmd->viewangles.z = float(roll);
    if (aaaa_enable)
    {
        UpdateAAAAKey();
        UpdateAAAATimer();
        p = GetAAAAPitch();
    }
    if (!g_pLocalPlayer->isFakeAngleCM)
        used_yaw = y;
    g_pLocalPlayer->bUseSilentAngles = true;
}

bool isEnabled()
{
    return *enable;
}

static InitRoutine fakelag_check([]() { yaw_fake.installChangeCallback([](settings::VariableBase<int> &, int after) { force_fakelag = after > 0 ? true : false; }); });
} // namespace hacks::shared::antiaim
