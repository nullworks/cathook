#pragma once

#include <optional>
#include <string>
#include <sdk.hpp>

namespace hacks::tf2::steamidstealer
{

class IDStorage
{
public:
    unsigned steamid;
    std::string reason;
    std::string name;
    IDStorage()
    {
        steamid = 0;
        reason  = "";
        name    = "";
    }
    IDStorage(unsigned steamid, std::string reason, std::string name = "")
    {
        this->steamid = steamid;
        this->reason  = reason;
        this->name    = name;
    }
};

std::optional<IDStorage> GetSteamID();
void SetSteamID(std::optional<IDStorage> steamid);
void SendNetMessage(INetMessage &msg);

} // namespace hacks::tf2::steamidstealer
