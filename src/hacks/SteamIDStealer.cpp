#include "common.hpp"
#include "SteamIDStealer.hpp"

namespace hacks::tf2::steamidstealer
{
static settings::String steamid_to_use("steamidstealer.steamid", "");

std::optional<IDStorage> stored;
std::optional<IDStorage> GetSteamID()
{
    return stored;
}

void SetSteamID(std::optional<IDStorage> steamid)
{
    stored = steamid;
}

void SendNetMessage(INetMessage &msg)
{
    // Client connection packet, modify steamid
    if (msg.GetType() == clc_ClientInfo)
    {
        uintptr_t *steamid = (uintptr_t *) (((uintptr_t) &msg) + 0x20);
        char *name         = (char *) (((uintptr_t) &msg) + 0x24);

        unsigned desired_steamid = 0;
        std::string desired_name = "";

        // Override if selected
        if (*steamid_to_use != "")
        {
            try
            {
                desired_steamid = std::stoul(*steamid_to_use);
            }
            catch (std::invalid_argument)
            {
                logging::Info("Bad Steamid provided!");
            }
        }
        else
        {
            // No steamid stored, return
            if (!stored)
                return;
            else
            {
                desired_steamid = stored->steamid;
                desired_name    = stored->name;
            }
        }
        logging::Info("Changing SteamID from %u to %u", *steamid, desired_steamid);
        *steamid = desired_steamid;
        // Update name too if desired
        if (desired_name != "")
            strncpy(name, desired_name.c_str(), std::min(32, (int) desired_name.length()));
    }
}
} // namespace hacks::tf2::steamidstealer
