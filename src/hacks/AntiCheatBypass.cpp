#include "common.hpp"

namespace hacks::tf2::antianticheat
{

settings::Boolean enabled("misc.antianticheat.enabled", "false");

// Always a good idea to spoof these
void SendNetMsg(INetMessage &msg)
{
    if (msg.GetType() == clc_RespondCvarValue)
    {
        CLC_RespondCvarValue *cvar_msg = (CLC_RespondCvarValue *) &msg;
        if (!cvar_msg->m_szCvarName)
            return;
        // Remove cat_ commands from cvar list
        if (!strcmp(cvar_msg->m_szCvarName, "cat") || !strncmp(cvar_msg->m_szCvarName, CON_PREFIX, strlen(CON_PREFIX)))
            cvar_msg->m_eStatusCode = EQueryCvarValueStatus::eQueryCvarValueStatus_CvarNotFound;
        // Remove the linux unique sdl_double_click_size
        else if (!strcmp(cvar_msg->m_szCvarName, "sdl_double_click_size"))
            cvar_msg->m_eStatusCode = EQueryCvarValueStatus::eQueryCvarValueStatus_CvarNotFound;
        // Spoof ourselves as Windows, bypassing Linux detection addons
        else if (std::string(cvar_msg->m_szCvarName) == "windows_speaker_config")
        {
            cvar_msg->m_eStatusCode = EQueryCvarValueStatus::eQueryCvarValueStatus_ValueIntact;
            cvar_msg->m_szCvarValue = "8";
        }
    }
}

// Currently unused, may be useful for stuff modifying cmdnum though.
bool CanSetCmdNum(int new_cmdnum)
{
    if (!enabled)
        return true;
    if (new_cmdnum - current_late_user_cmd->command_number >= 64)
        return false;
    return true;
}

static void CreateMoveLate()
{
    if (!enabled)
        return;
    static int silent_ticks = 0;
    // "Silent" detection mostly just checks if cursor was where it was 2 ticks ago while having moved 1 tick before
    if (silent_ticks == 1 && !g_pLocalPlayer->bUseSilentAngles)
        current_late_user_cmd->viewangles += Vector(RandomFloat(-1.0f, 1.0f));

    silent_ticks++;
    if (!g_pLocalPlayer->bUseSilentAngles)
        silent_ticks = 0;
}

static InitRoutine init([]() { EC::Register(EC::CreateMoveLate, CreateMoveLate, "acb_cml"); });
} // namespace hacks::tf2::antianticheat
