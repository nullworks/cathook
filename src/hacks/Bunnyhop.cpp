/*
 * HBunnyhop.cpp
 *
 *  Created on: Oct 6, 2016
 *      Author: nullifiedcat
 */

#include <settings/Bool.hpp>
#include "common.hpp"

namespace hacks::shared::bunnyhop
{
static settings::Boolean enable{ "bunnyhop.enable", "false" };
static settings::Int bhop_chance{ "bunnyhop.chance", "100" };

// Var for user settings

static int ticks_last_jump = 0;
// static int perfect_jumps = 0;

// Function called by game for movement
static void CreateMove()
{
    if (!CE_GOOD(LOCAL_E) || !LOCAL_E->m_bAlivePlayer() || CE_BAD(LOCAL_W))
        return;
    // Check user settings if bhop is enabled
    if (!enable)
        return;

    // Check if there is usercommands
    if (!current_user_cmd->command_number)
        return;

    // Bhop likelihood
    if (UniformRandomInt(0, 99) > *bhop_chance)
        return;

    // var for "if on ground" from the flags netvar
    bool ground = CE_INT(g_pLocalPlayer->entity, netvar.iFlags) & (1 << 0);
    // Var for if the player is pressing jump
    bool jump = (current_user_cmd->buttons & IN_JUMP);

    // Check if player is not on the ground and player is holding their jump key
    if (!ground && jump)
    {
        // If the ticks since last jump are greater or equal to 9, then force
        // the player to stop jumping The bot disables jump untill player hits
        // the ground or lets go of jump
        if (ticks_last_jump++ >= 9)
        {
            current_user_cmd->buttons = current_user_cmd->buttons & ~IN_JUMP;
        }
    }

    // If the players jump cmd has been used, then we reset our var
    if (!jump)
        ticks_last_jump = 0;
}
static InitRoutine EC([]() { EC::Register(EC::CreateMove_NoEnginePred, CreateMove, "Bunnyhop", EC::early); });
} // namespace hacks::shared::bunnyhop
