/*
 *  Credits to UNKN0WN
 */
#include "common.hpp"
#include "DetourHook.hpp"
#include "PlayerTools.hpp"
#include <boost/algorithm/string.hpp>
#include <regex>
#include <locale>
#include <codecvt>
#include <string>

#if !ENFORCE_STREAM_SAFETY
namespace hacks::tf2::killfeed
{
static settings::Boolean enable{ "visual.killfeedcolor.enable", "true" };
static settings::Boolean sort_names{ "visual.killfeedcolor.sort-names", "false" };

static DetourHook drawtext_detour;

typedef void (*DrawText_t)(int *_this, int x, int y, vgui::HFont hFont, Color clr, const wchar_t *szText);

void DrawText_hook(int *_this, int x, int y, vgui::HFont hFont, Color clr, const wchar_t *szText)
{
    if (isHackActive() && enable)
    {
        // Convert wchar_t * to std::string for easier manipulation
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::string normal_string = converter.to_bytes(szText);

        std::vector<std::string> names_array;

        // Since we have no access to boost's regex split, we need to replace our sequence
        // With a single character.
        // Luckily names can never have a % in them, so i will use that as the delimeter.

        static std::regex r("( \\+ |, )");
        normal_string = std::regex_replace(normal_string, r, "%");

        // Split at assists
        boost::algorithm::split(names_array, normal_string, boost::is_any_of("%"));

        // Sort all player names by priority
        static int last_sort_tickcount = 0;
        static boost::unordered_flat_map<std::string, int> player_names;
        // Cache names for every tick
        if (last_sort_tickcount != tickcount)
        {
            player_names.clear();
            for (int i = 1; i < g_IEngine->GetMaxClients(); ++i)
            {
                player_info_s player_info;
                if (GetPlayerInfo(i, &player_info))
                    player_names[player_info.name] = i;
            }
        }

        // Stored list of valid players in text
        std::vector<player_info_s> displayed_players;

        // Get entities by name
        for (auto &name : names_array)
        {
            player_info_s pinfo;
            if (player_names[name] != 0 && GetPlayerInfo(player_names[name], &pinfo))
                displayed_players.push_back(pinfo);
        }

        if (sort_names)
        {
            // Sort by state so we display the most important color
            std::sort(displayed_players.begin(), displayed_players.end(),
                      [](player_info_s a, player_info_s b)
                      {
                          auto a_data = playerlist::AccessData(a.friendsID);
                          auto b_data = playerlist::AccessData(b.friendsID);

                          // Function to get priority of a state, RAGE > CAT/FRIEND > DEFAULT
                          static auto getPriority = [](playerlist::k_EState state)
                          {
                              switch (state)
                              {
                              case playerlist::k_EState::DEFAULT:
                                  return 0;
                              case playerlist::k_EState::CAT:
                              case playerlist::k_EState::FRIEND:
                                  return 2;
                              case playerlist::k_EState::RAGE:
                                  return 3;
                              default:
                                  return 1;
                              }
                          };

                          return getPriority(a_data.state) > getPriority(b_data.state);
                      });
        }

        if (!displayed_players.empty())
        {
            auto original = (DrawText_t) drawtext_detour.GetOriginalFunc();
            for (auto &player : displayed_players)
            {
                Color draw_clr      = clr;
                rgba_t player_color = playerlist::Color(player.friendsID);
                if (player_color != colors::empty)
                    draw_clr.SetColor(player_color.r * 255, player_color.g * 255, player_color.b * 255, 255);

                // Draw text
                std::wstring wide = converter.from_bytes(player.name);
                original(_this, x, y, hFont, draw_clr, wide.c_str());

                int width, height = 0;
                g_ISurface->GetTextSize(hFont, wide.c_str(), width, height);
                x += width;

                // Draw seperator if this is not the last entry
                if (&player != &displayed_players.back())
                {
                    static std::wstring seperator = L", ";
                    original(_this, x, y, hFont, clr, seperator.c_str());
                    g_ISurface->GetTextSize(hFont, seperator.c_str(), width, height);
                    x += width;
                }
            }
            drawtext_detour.RestorePatch();
        }
        else
        {
            auto original = (DrawText_t) drawtext_detour.GetOriginalFunc();
            original(_this, x, y, hFont, clr, szText);
            drawtext_detour.RestorePatch();
        }
    }
    else
    {
        auto original = (DrawText_t) drawtext_detour.GetOriginalFunc();
        original(_this, x, y, hFont, clr, szText);
        drawtext_detour.RestorePatch();
    }
}

static InitRoutine init(
    []
    {
        auto drawtext_addr = gSignatures.GetClientSignature("55 89 E5 57 56 53 83 EC 1C A1 ? ? ? ? 8B 4D");
        drawtext_detour.Init(drawtext_addr, (void *) DrawText_hook);

        EC::Register(
            EC::Shutdown, []() { drawtext_detour.Shutdown(); }, "shutdown_kf");
    });
} // namespace hacks::tf2::killfeed
#endif
