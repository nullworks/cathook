/*  This file is part of Cathook.

    Cathook is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Cathook is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Cathook.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "common.hpp"
#include "ChatCommands.hpp"
#include "MiscTemporary.hpp"
#include <iostream>

namespace hacks::shared::ChatCommands
{

static settings::Boolean enabled("chatcommands.enabled", "false");

struct ChatCommand
{
    void addcommand(std::string cmd)
    {
        commands.push_back(cmd);
    }
    bool readFile(std::string filename)
    {
        auto stream = std::ifstream(paths::getDataPath("/chatcommands/" + filename));
        if (!stream)
            return false;
        for (std::string line; getline(stream, line);)
        {
            commands.push_back(line);
        }
        return true;
    }
    const std::vector<std::string> &getCommands()
    {
        return commands;
    }

private:
    std::vector<std::string> commands;
};

static boost::unordered_flat_map<std::string, ChatCommand> commands;

void handleChatMessage(std::string message, int senderid)
{
    if (!enabled)
        return;
    logging::Info("%s, %d", message.c_str(), senderid);

    std::string prefix;
    std::string all_params;
    std::string::size_type space_pos = message.find_first_of(" ");
    if (space_pos == message.npos)
    {
        prefix     = message;
        all_params = "";
    }
    else
    {
        prefix     = message.substr(0, space_pos);
        all_params = message.substr(space_pos + 1);
        ReplaceString(all_params, "'", "`");
        ReplaceString(all_params, "\"", "`");
        ReplaceString(all_params, ";", ",");
    }
    logging::Info("Prefix: %s", prefix.c_str());

    auto ccmd = commands.find(prefix);
    // Check if its a registered command
    if (ccmd == commands.end())
        return;
    for (auto cmd : ccmd->second.getCommands())
    {
        player_info_s localinfo{};
        if (!GetPlayerInfo(g_pLocalPlayer->entity_idx, &localinfo))
            return;
        player_info_s senderinfo{};
        if (!GetPlayerInfo(senderid, &senderinfo))
            return;

        ReplaceString(cmd, "%myname%", std::string(localinfo.name));
        ReplaceString(cmd, "%name%", std::string(senderinfo.name));
        ReplaceString(cmd, "%steamid%", std::to_string(senderinfo.friendsID));
        ReplaceString(cmd, "%all_params%", all_params);
        g_IEngine->ClientCmd_Unrestricted(cmd.c_str());
    }
}

static CatCommand chatcommands_add("chatcommands_add", "chatcommands_add <chat command> <command>",
                                   [](const CCommand &args)
                                   {
                                       if (args.ArgC() != 3)
                                       {
                                           g_ICvar->ConsoleColorPrintf(MENU_COLOR, "usage: chatcommands_add <chat command> <command>\n");
                                           return;
                                       }
                                       std::string prefix = args.Arg(1);
                                       if (prefix.find(' ') != prefix.npos)
                                       {
                                           g_ICvar->ConsoleColorPrintf(MENU_COLOR, "The chat command mustn't contain spaces!\n");
                                           return;
                                       }
                                       std::string command = args.Arg(2);

                                       auto &chatcommand = commands[prefix];

                                       logging::Info("%s: %s", prefix.c_str(), command.c_str());
                                       chatcommand.addcommand(command);
                                   });

static CatCommand chatcommands_file("chatcommands_file", "chatcommands_add <chat command> <filename in " + paths::getDataPath() + "/chatcommands>",
                                    [](const CCommand &args)
                                    {
                                        if (args.ArgC() != 3)
                                        {
                                            g_ICvar->ConsoleColorPrintf(MENU_COLOR, ("usage: chatcommands_add <chat command> <filename in " + paths::getDataPath() + "/chatcommands>\n").c_str());
                                            return;
                                        }
                                        std::string prefix = args.Arg(1);
                                        if (prefix.find(' ') != prefix.npos)
                                        {
                                            g_ICvar->ConsoleColorPrintf(MENU_COLOR, "The chat command mustn't contain spaces!\n");
                                            return;
                                        }
                                        std::string file = args.Arg(2);

                                        auto &chatcommand = commands[prefix];

                                        if (!chatcommand.readFile(file))
                                        {
                                            g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Couldn't open the file!\n");
                                            return;
                                        }
                                        logging::Info("%s: %s", prefix.c_str(), file.c_str());
                                    });

static CatCommand chatcommands_reset_all("chatcommands_reset_all", "Clears all chatcommands",
                                         [](const CCommand &args)
                                         {
                                             commands.clear();
                                             g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Chat commands cleared!\n");
                                         });

} // namespace hacks::shared::ChatCommands
