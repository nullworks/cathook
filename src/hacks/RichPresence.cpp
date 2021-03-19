/*
    This file is part of Cathook.

    Cathook is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Cathook is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Cathook. If not, see <https://www.gnu.org/licenses/>.
*/

// Codeowners: aUniqueUser

#include "common.hpp"

namespace hacks::tf2::richpresence
{

/* meme feature for editing the steam friendsui rich presence */
static settings::Boolean rich_presence{ "misc.rich-presence.enabled", "false" };
static settings::String rich_presence_file{ "misc.rich-presence.file", "rich_presence.txt" };
static settings::Int rich_presence_party_size{ "misc.rich-presence.party_size", "1337" };
static settings::Int rich_presence_change_delay{ "misc.rich-presence.delay", "5000" };

static std::vector<std::string> rich_presence_text;
static size_t current_presence_idx = 0;
static Timer rich_presence_timer;

void PresenceReload(std::string after)
{
    rich_presence_text.clear();
    current_presence_idx = 0;
    if (!after.empty())
    {
        static TextFile teamspam;
        if (teamspam.TryLoad(after))
            rich_presence_text = teamspam.lines;
    }
}

void Paint()
{
    if (!rich_presence || !rich_presence_timer.test_and_set(*rich_presence_change_delay))
        return;

    if (!rich_presence_text.empty())
    {
        g_ISteamFriends->SetRichPresence("steam_display", "#TF_RichPresence_Display");
        g_ISteamFriends->SetRichPresence("state", "PlayingMatchGroup");
        g_ISteamFriends->SetRichPresence("matchgrouploc", "SpecialEvent");

        if (current_presence_idx >= rich_presence_text.size())
            current_presence_idx = 0;

        g_ISteamFriends->SetRichPresence("currentmap", rich_presence_text[current_presence_idx].c_str());
        current_presence_idx++;
    }
    g_ISteamFriends->SetRichPresence("steam_player_group_size", std::to_string(*rich_presence_party_size + 1).c_str());
}

static CatCommand reload_presence("presence_reload", "Reload rich presence file", []() { PresenceReload(*rich_presence_file); });
static CatCommand remove_richpresence("remove_presence", "Remove rich presence", []() { g_ISteamFriends->ClearRichPresence(); });

static InitRoutine init([] {
    rich_presence_file.installChangeCallback([](settings::VariableBase<std::string> &, std::string after) { PresenceReload(after); });
    EC::Register(EC::Paint, Paint, "paint_rich_presence");
});

} // namespace hacks::tf2::richpresence
