/*
  Created on 29.07.18.
*/
#include <menu/GuiInterface.hpp>

#include <menu/menu/Menu.hpp>
#include <drawing.hpp>
#include <menu/menu/special/SettingsManagerList.hpp>
#include <menu/menu/special/ConfigsManagerList.hpp>
#include <menu/menu/special/PlayerListController.hpp>
#include <hack.hpp>
#include <common.hpp>

settings::Button open_gui_button{ "visual.open-gui-button", "Insert" };

static bool init_done{ false };

static std::unique_ptr<zerokernel::special::PlayerListController> controller{ nullptr };

static zerokernel::special::PlayerListData createPlayerListData(int userid)
{
    zerokernel::special::PlayerListData data{};
    auto idx = g_IEngine->GetPlayerForUserID(userid);
    player_info_s info{};
    g_IEngine->GetPlayerInfo(idx, &info);
    data.classId = g_pPlayerResource->getClass(idx);
    data.teamId  = g_pPlayerResource->getTeam(idx) - 1;
    data.dead    = !g_pPlayerResource->isAlive(idx);
    data.steam   = info.friendsID;
    data.state   = playerlist::k_pszNames[static_cast<int>(playerlist::AccessData(info.friendsID).state)];
    data.name    = info.name;
    return data;
}

class PlayerListEventListener : public IGameEventListener
{
public:
    void FireGameEvent(KeyValues *event) override
    {
        if (!controller)
            return;

        auto userid = event->GetInt("userid");
        if (!userid)
            return;

        std::string name = event->GetName();
        if (name == "player_connect_client")
        {
            logging::Info("addPlayer %d", userid);
            controller->addPlayer(userid, createPlayerListData(userid));
        }
        else if (name == "player_disconnect")
        {
            // logging::Info("removePlayer %d", userid);
            controller->removePlayer(userid);
        }
        else if (name == "player_team")
        {
            // logging::Info("updatePlayerTeam %d", userid);
            controller->updatePlayerTeam(userid, event->GetInt("team") - 1);
        }
        else if (name == "player_changeclass")
        {
            // logging::Info("updatePlayerClass %d", userid);
            controller->updatePlayerClass(userid, event->GetInt("class"));
        }
        else if (name == "player_changename")
        {
            // logging::Info("updatePlayerName %d", userid);
            controller->updatePlayerName(userid, event->GetString("newname"));
        }
        else if (name == "player_death")
        {
            // logging::Info("updatePlayerLifeState %d", userid);
            controller->updatePlayerLifeState(userid, true);
        }
        else if (name == "player_spawn")
        {
            // logging::Info("updatePlayerLifeState %d", userid);
            controller->updatePlayerLifeState(userid, false);
        }
    }
};

static PlayerListEventListener listener{};

static void initPlayerlist()
{
    auto pl = dynamic_cast<zerokernel::Table *>(zerokernel::Menu::instance->wm->getElementById("special-player-list"));
    if (pl)
    {
        controller = std::make_unique<zerokernel::special::PlayerListController>(*pl);
        controller->setKickButtonCallback([](int uid) { hack::command_stack().push(format("callvote kick ", uid)); });
        controller->setOpenSteamCallback([](unsigned steam) {
            CSteamID id{};
            id.Set(steam, EUniverse::k_EUniversePublic, EAccountType::k_EAccountTypeIndividual);
            g_ISteamFriends->ActivateGameOverlayToUser("steamid", id);
        });
        controller->setChangeStateCallback([](unsigned steam, int userid) {
            auto &pl = playerlist::AccessData(steam);
            for (unsigned i = 0; i < playerlist::k_arrGUIStates.size() - 1; i++)
            {
                if (pl.state == playerlist::k_arrGUIStates.at(i).first)
                {
                    pl.state = playerlist::k_arrGUIStates.at(i + 1).first;
                    controller->updatePlayerState(userid, playerlist::k_Names[static_cast<size_t>(pl.state)]);
                    return;
                }
            }
            pl.state = playerlist::k_arrGUIStates.front().first;
            controller->updatePlayerState(userid, playerlist::k_Names[static_cast<size_t>(pl.state)]);
        });
    }
    else
    {
        logging::Info("PlayerList element not found\n");
    }
}

static void load()
{
    zerokernel::Menu::instance->loadFromFile(paths::getDataPath("/menu"), "menu.xml");

    zerokernel::Container *sv = dynamic_cast<zerokernel::Container *>(zerokernel::Menu::instance->wm->getElementById("special-variables"));
    if (sv)
    {
        zerokernel::special::SettingsManagerList list(*sv);
        list.construct();
        printf("SV found\n");
    }
    
    zerokernel::Container *cl = dynamic_cast<zerokernel::Container *>(zerokernel::Menu::instance->wm->getElementById("cfg-list"));
    if (cl)
    {
        zerokernel::special::ConfigsManagerList list(*cl);
        list.construct();
        printf("CL found\n");
    }
    
    initPlayerlist();

    zerokernel::Menu::instance->update();
    zerokernel::Menu::instance->setInGame(true);
}

static CatCommand reload("gui_reload", "Reload", []() { load(); });

void gui::init()
{
    zerokernel::Menu::init(draw::width, draw::height);
    g_IGameEventManager->AddListener(&listener, false);

    load();

    init_done = true;
}

void gui::shutdown()
{
    if (init_done)
        g_IGameEventManager->RemoveListener(&listener);
}

void gui::draw()
{
    if (!init_done)
        return;

    zerokernel::Menu::instance->update();
    zerokernel::Menu::instance->render();
}
static Timer update_players{};
bool gui::handleSdlEvent(SDL_Event *event)
{
    if (!zerokernel::Menu::instance)
        return false;
    if (controller && CE_GOOD(LOCAL_E) && update_players.test_and_set(10000))
    {
        controller->removeAll();
        for (auto i = 1; i <= MAX_PLAYERS; ++i)
        {
            player_info_s info{};
            if (g_IEngine->GetPlayerInfo(i, &info))
            {
                controller->addPlayer(info.userID, createPlayerListData(info.userID));
            }
        }
    }
    if (event->type == SDL_KEYDOWN)
    {
        if (event->key.keysym.scancode == SDL_GetScancodeFromKey((*open_gui_button).keycode))
        {
            // logging::Info("GUI open button pressed");
            zerokernel::Menu::instance->setInGame(!zerokernel::Menu::instance->isInGame());
            if (!zerokernel::Menu::instance->isInGame())
            {
                g_ISurface->UnlockCursor();
                g_ISurface->SetCursorAlwaysVisible(true);
            }
            else
            {
                g_ISurface->LockCursor();
                g_ISurface->SetCursorAlwaysVisible(false);
                // Ensure we don't snap after closing the menu by deactivating and
                // reactivating the mouse, which causes it to fully reset
                g_IInput->DeactivateMouse();
                g_IInput->ActivateMouse();
            }
            return true;
        }
    }
    zerokernel::Menu::instance->handleSdlEvent(event);
    if (!zerokernel::Menu::instance->isInGame() && (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_TEXTINPUT || event->type == SDL_KEYDOWN))
        return true;
    else
        return false;
}

void gui::onLevelLoad()
{
    if (controller)
    {
        controller->removeAll();
        for (auto i = 1; i < MAX_PLAYERS; ++i)
        {
            player_info_s info{};
            if (g_IEngine->GetPlayerInfo(i, &info))
            {
                controller->addPlayer(info.userID, createPlayerListData(info.userID));
            }
        }
    }
}
