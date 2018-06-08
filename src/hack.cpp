/*
 * hack.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#include <visual/SDLHooks.hpp>
#include "hack.hpp"
#include "common.hpp"
#include "MiscTemporary.hpp"
#include "xorstring.h"

#include <hacks/hacklist.hpp>

#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

#include "CDumper.hpp"
#include "version.h"

/*
 *  Credits to josh33901 aka F1ssi0N for butifel F1Public and Darkstorm 2015
 * Linux
 */

bool hack::shutdown    = false;
bool hack::initialized = false;

const std::string &hack::GetVersion()
{
    static std::string version(XORSTR("Unknown Version"));
    static bool version_set = false;
    if (version_set)
        return version;
#if defined(GIT_COMMIT_HASH) && defined(GIT_COMMIT_DATE)
    version = "Version: #" GIT_COMMIT_HASH " " GIT_COMMIT_DATE;
#endif
    version_set = true;
    return version;
}

const std::string &hack::GetType()
{
    static std::string version(XORSTR("Unknown Type"));
    static bool version_set = false;
    if (version_set)
        return version;
    version = XORSTR("");
#if not ENABLE_IPC
    version += XORSTR(" NOIPC");
#endif
#if not ENABLE_GUI
    version += XORSTR(" NOGUI");
#else
    version += " GUI";
#endif

#ifndef DYNAMIC_CLASSES

#ifdef GAME_SPECIFIC
    version += " GAME " TO_STRING(GAME);
#else
    version += XORSTR(" UNIVERSAL");
#endif

#else
    version += XORSTR(" DYNAMIC");
#endif

#if not ENABLE_VISUALS
    version += XORSTR(" NOVISUALS");
#endif

    version = version.substr(1);
    version_set = true;
    return version;
}

std::mutex hack::command_stack_mutex;
std::stack<std::string> &hack::command_stack()
{
    static std::stack<std::string> stack;
    return stack;
}

#if ENABLE_VISUALS /* Why would we need colored chat stuff in textmode?        \
                         */
#define red 184, 56, 59, 255
#define blu 88, 133, 162, 255
static CatVar cat_event_hurt(CV_SWITCH, XORSTR("cat_event_hurt"), XORSTR("1"),
                             XORSTR("Enable OnHurt Event"),
                             XORSTR("Disable if your chat gets spammed with \"blah "
                             "damaged blah down to blah hp\""));
class AdvancedEventListener : public IGameEventListener
{
public:
    virtual void FireGameEvent(KeyValues *event)
    {
        if (!event_log)
            return;
        const char *name = event->GetName();
        if (!strcmp(name, XORSTR("player_connect_client")))
            PrintChat(XORSTR("\x07%06X%s\x01 \x07%06X%s\x01 joining"), 0xa06ba0,
                      event->GetString(XORSTR("name")), 0x914e65,
                      event->GetString(XORSTR("networkid")));
        else if (!strcmp(name, XORSTR("player_activate")))
        {
            int uid    = event->GetInt(XORSTR("userid"));
            int entity = g_IEngine->GetPlayerForUserID(uid);
            player_info_s info;
            if (g_IEngine->GetPlayerInfo(entity, &info))
                PrintChat(XORSTR("\x07%06X%s\x01 connected"), 0xa06ba0, info.name);
        }
        else if (!strcmp(name, XORSTR("player_disconnect")))
        {
            CachedEntity *player =
                ENTITY(g_IEngine->GetPlayerForUserID(event->GetInt(XORSTR("userid"))));
            PrintChat(XORSTR("\x07%06X%s\x01 \x07%06X%s\x01 disconnected"),
                      colors::chat::team(player->m_iTeam()),
                      event->GetString(XORSTR("name")), 0x914e65,
                      event->GetString(XORSTR("networkid")));
        }
        else if (!strcmp(name, XORSTR("player_team")))
        {
            if (event->GetBool(XORSTR("disconnect")) != 1)
            {
                int oteam           = event->GetInt(XORSTR("oldteam"));
                int nteam           = event->GetInt(XORSTR("team"));
                const char *oteam_s = teamname(oteam);
                const char *nteam_s = teamname(nteam);
                PrintChat(XORSTR("\x07%06X%s\x01 changed team (\x07%06X%s\x01 -> "),
                          XORSTR("\x07%06X%s\x01)"),
                          0xa06ba0, event->GetString(XORSTR("name")),
                          colors::chat::team(oteam), oteam_s,
                          colors::chat::team(nteam), nteam_s);
            }
        }
        else if (!strcmp(name, XORSTR("player_hurt")))
        {
            int victim   = event->GetInt(XORSTR("userid"));
            int attacker = event->GetInt(XORSTR("attacker"));
            int health   = event->GetInt(XORSTR("health"));
            player_info_s kinfo;
            player_info_s vinfo;
            g_IEngine->GetPlayerInfo(g_IEngine->GetPlayerForUserID(victim),
                                     &vinfo);
            g_IEngine->GetPlayerInfo(g_IEngine->GetPlayerForUserID(attacker),
                                     &kinfo);
            CachedEntity *vic = ENTITY(g_IEngine->GetPlayerForUserID(victim));
            CachedEntity *att = ENTITY(g_IEngine->GetPlayerForUserID(attacker));
            PrintChat(
                XORSTR("\x07%06X%s\x01 hurt \x07%06X%s\x01 down to \x07%06X%d\x01hp"),
                colors::chat::team(att->m_iTeam()), kinfo.name,
                colors::chat::team(vic->m_iTeam()), vinfo.name, 0x2aaf18,
                health);
        }
        else if (!strcmp(name, XORSTR("player_death")))
        {
            int victim   = event->GetInt(XORSTR("userid"));
            int attacker = event->GetInt(XORSTR("attacker"));
            player_info_s kinfo;
            player_info_s vinfo;
            g_IEngine->GetPlayerInfo(g_IEngine->GetPlayerForUserID(victim),
                                     &vinfo);
            g_IEngine->GetPlayerInfo(g_IEngine->GetPlayerForUserID(attacker),
                                     &kinfo);
            CachedEntity *vic = ENTITY(g_IEngine->GetPlayerForUserID(victim));
            CachedEntity *att = ENTITY(g_IEngine->GetPlayerForUserID(attacker));
            PrintChat(XORSTR("\x07%06X%s\x01 killed \x07%06X%s\x01"),
                      colors::chat::team(att->m_iTeam()), kinfo.name,
                      colors::chat::team(vic->m_iTeam()), vinfo.name);
        }
        else if (!strcmp(name, XORSTR("player_spawn")))
        {
            int id = event->GetInt(XORSTR("userid"));
            player_info_s info;
            g_IEngine->GetPlayerInfo(g_IEngine->GetPlayerForUserID(id), &info);
            CachedEntity *player = ENTITY(g_IEngine->GetPlayerForUserID(id));
            PrintChat(XORSTR("\x07%06X%s\x01 (re)spawned"),
                      colors::chat::team(player->m_iTeam()), info.name);
        }
        else if (!strcmp(name, XORSTR("player_changeclass")))
        {
            int id = event->GetInt(XORSTR("userid"));
            player_info_s info;
            g_IEngine->GetPlayerInfo(g_IEngine->GetPlayerForUserID(id), &info);
            CachedEntity *player = ENTITY(g_IEngine->GetPlayerForUserID(id));
            PrintChat(XORSTR("\x07%06X%s\x01 changed to \x07%06X%s\x01"),
                      colors::chat::team(player->m_iTeam()), info.name,
                      0xa06ba0, classname(event->GetInt(XORSTR("class"))));
        }
        else if (!strcmp(name, XORSTR("vote_cast")))
        {
            int vote_option = event->GetInt(XORSTR("vote_option"));
            int team        = event->GetInt(XORSTR("team"));
            int idx         = event->GetInt(XORSTR("entityid"));
            player_info_s info;
            const char *team_s = teamname(team);
            g_IEngine->GetPlayerInfo(idx, &info);
            PrintChat(
                XORSTR("\x07%06X%s\x01 Voted \x07%06X%d\x01 on team \x07%06X%s\x01"),
                colors::chat::team(team), info.name, colors::chat::team(team),
                vote_option, colors::chat::team(team), team_s);
        };
    }
};

AdvancedEventListener adv_event_listener{};

#endif /* TEXTMODE */

void hack::ExecuteCommand(const std::string command)
{
    std::lock_guard<std::mutex> guard(hack::command_stack_mutex);
    hack::command_stack().push(command);
}

ConCommand *hack::c_Cat = 0;

void hack::CC_Cat(const CCommand &args)
{
    g_ICvar->ConsoleColorPrintf(Color(255, 255, 255, 255), XORSTR("cathook"));
    g_ICvar->ConsoleColorPrintf(Color(0, 0, 255, 255), XORSTR(" by "));
    g_ICvar->ConsoleColorPrintf(Color(255, 0, 0, 255), XORSTR("nullifiedcat\n"));
}

void hack::Initialize()
{
    signal(SIGPIPE, SIG_IGN);
    time_injected = time(nullptr);
/*passwd *pwd   = getpwuid(getuid());
char *logname = strfmt(XORSTR("/tmp/cathook-game-stdout-%s-%u.log"), pwd->pw_name,
time_injected);
freopen(logname, XORSTR("w"), stdout);
free(logname);
logname = strfmt(XORSTR("/tmp/cathook-game-stderr-%s-%u.log"), pwd->pw_name,
time_injected);
freopen(logname, XORSTR("w"), stderr);
free(logname);*/
// Essential files must always exist, except when the game is running in text
// mode.
#if ENABLE_VISUALS

    {
        std::vector<std::string> essential = { XORSTR("menu.json"),
                                               XORSTR("fonts/tf2build.ttf") };
        for (const auto &s : essential)
        {
            std::ifstream exists(DATA_PATH "/" + s, std::ios::in);
            if (not exists)
            {
                Error(("Missing essential file: " + s +
                        "/%s\nYou MUST run install-data script to finish "
                       "installation")
                          .c_str(),
                      s.c_str());
            }
        }
    }

#endif /* TEXTMODE */

    logging::Info(XORSTR("Initializing..."));
    srand(time(0));
    sharedobj::LoadAllSharedObjects();
    CreateInterfaces();
    CDumper dumper;
    dumper.SaveDump();
    logging::Info(XORSTR("Is TF2? %d"), IsTF2());
    logging::Info(XORSTR("Is TF2C? %d"), IsTF2C());
    logging::Info(XORSTR("Is HL2DM? %d"), IsHL2DM());
    logging::Info(XORSTR("Is CSS? %d"), IsCSS());
    logging::Info(XORSTR("Is TF? %d"), IsTF());
    InitClassTable();

    BeginConVars();
    hack::c_Cat = CreateConCommand(CON_NAME, &hack::CC_Cat, XORSTR("Info"));
    g_Settings.Init();
    EndConVars();

#if ENABLE_VISUALS
    draw::Initialize();
#if ENABLE_GUI

    g_pGUI = new CatGUI();
    g_pGUI->Setup();

#endif

#endif /* TEXTMODE */

    gNetvars.init();
    InitNetVars();
    g_pLocalPlayer    = new LocalPlayer();
    g_pPlayerResource = new TFPlayerResource();

    uintptr_t *clientMode = 0;
    // Bad way to get clientmode.
    // FIXME [MP]?
    while (!(
        clientMode = **(
            uintptr_t ***) ((uintptr_t)((*(void ***) g_IBaseClient)[10]) + 1)))
    {
        usleep(10000);
    }
    hooks::clientmode.Set((void *) clientMode);
    hooks::clientmode.HookMethod(HOOK_ARGS(CreateMove));
#if ENABLE_VISUALS
    hooks::clientmode.HookMethod(HOOK_ARGS(OverrideView));
#endif
    hooks::clientmode.HookMethod(HOOK_ARGS(LevelInit));
    hooks::clientmode.HookMethod(HOOK_ARGS(LevelShutdown));
    hooks::clientmode.Apply();

    hooks::clientmode4.Set((void *) (clientMode), 4);
    hooks::clientmode4.HookMethod(HOOK_ARGS(FireGameEvent));
    hooks::clientmode4.Apply();

    hooks::client.Set(g_IBaseClient);
    hooks::client.HookMethod(HOOK_ARGS(DispatchUserMessage));
#if ENABLE_VISUALS
    hooks::client.HookMethod(HOOK_ARGS(FrameStageNotify));
    hooks::client.HookMethod(HOOK_ARGS(IN_KeyEvent));
#endif
    hooks::client.Apply();

#if ENABLE_VISUALS
    hooks::vstd.Set((void *) g_pUniformStream);
    hooks::vstd.HookMethod(HOOK_ARGS(RandomInt));
    hooks::vstd.Apply();

    hooks::panel.Set(g_IPanel);
    hooks::panel.HookMethod(hooked_methods::methods::PaintTraverse,
                            offsets::PaintTraverse(),
                            &hooked_methods::original::PaintTraverse);
    hooks::panel.Apply();
#endif

    hooks::input.Set(g_IInput);
    hooks::input.HookMethod(HOOK_ARGS(GetUserCmd));
    hooks::input.Apply();
#if ENABLE_VISUALS
    hooks::modelrender.Set(g_IVModelRender);
    hooks::modelrender.HookMethod(HOOK_ARGS(DrawModelExecute));
    hooks::modelrender.Apply();
#endif
    hooks::enginevgui.Set(g_IEngineVGui);
    hooks::enginevgui.HookMethod(HOOK_ARGS(Paint));
    hooks::enginevgui.Apply();

    hooks::engine.Set(g_IEngine);
    hooks::engine.HookMethod(HOOK_ARGS(IsPlayingTimeDemo));
    hooks::engine.Apply();

    hooks::eventmanager2.Set(g_IEventManager2);
    hooks::eventmanager2.HookMethod(HOOK_ARGS(FireEvent));
    hooks::eventmanager2.HookMethod(HOOK_ARGS(FireEventClientSide));
    hooks::eventmanager2.Apply();

    hooks::steamfriends.Set(g_ISteamFriends);
    hooks::steamfriends.HookMethod(HOOK_ARGS(GetFriendPersonaName));
    hooks::steamfriends.Apply();

#if ENABLE_NULL_GRAPHICS
    g_IMaterialSystem->SetInStubMode(true);
    IF_GAME(IsTF2())
    {
        logging::Info(XORSTR("Graphics Nullified"));
        logging::Info(XORSTR("The game will crash"));
        // TODO offsets::()?
        hooks::materialsystem.Set((void *) g_IMaterialSystem);
        uintptr_t base = *(uintptr_t *) (g_IMaterialSystem);
        hooks::materialsystem.HookMethod((void *) ReloadTextures_null_hook, 70);
        hooks::materialsystem.HookMethod((void *) ReloadMaterials_null_hook,
                                         71);
        hooks::materialsystem.HookMethod((void *) FindMaterial_null_hook, 73);
        hooks::materialsystem.HookMethod((void *) FindTexture_null_hook, 81);
        hooks::materialsystem.HookMethod((void *) ReloadFilesInList_null_hook,
                                         121);
        hooks::materialsystem.HookMethod((void *) FindMaterialEx_null_hook,
                                         123);
        hooks::materialsystem.Apply();
        // hooks::materialsystem.HookMethod();
    }
#endif
#if not LAGBOT_MODE
    // FIXME [MP]
    hacks::shared::killsay::Init();
    hacks::shared::announcer::init();
    hacks::tf2::killstreak::init();
#endif
    hacks::shared::catbot::init();
    logging::Info(XORSTR("Hooked!"));
    velocity::Init();
    playerlist::Load();

#if ENABLE_VISUALS

    InitStrings();
#if ENABLE_GUI
    // cat_reloadscheme to load imgui
    hack::command_stack().push(XORSTR("cat_reloadscheme"));
#endif
#ifndef FEATURE_EFFECTS_DISABLED
    if (g_ppScreenSpaceRegistrationHead && g_pScreenSpaceEffects)
    {
        effect_chams::g_pEffectChams = new CScreenSpaceEffectRegistration(
            XORSTR("_cathook_chams"), &effect_chams::g_EffectChams);
        g_pScreenSpaceEffects->EnableScreenSpaceEffect(XORSTR("_cathook_chams"));
        effect_chams::g_EffectChams.Init();
        effect_glow::g_pEffectGlow = new CScreenSpaceEffectRegistration(
            XORSTR("_cathook_glow"), &effect_glow::g_EffectGlow);
        g_pScreenSpaceEffects->EnableScreenSpaceEffect(XORSTR("_cathook_glow"));
    }
    logging::Info(XORSTR("SSE enabled.."));
#endif
    sdl_hooks::applySdlHooks();
    logging::Info(XORSTR("SDL hooking done"));
    g_IGameEventManager->AddListener(&adv_event_listener, false);

#endif /* TEXTMODE */
#if not LAGBOT_MODE
    hacks::shared::anticheat::Init();
    hacks::tf2::healarrow::Init();
#endif
#if ENABLE_VISUALS
#ifndef FEATURE_FIDGET_SPINNER_ENABLED
    InitSpinner();
    logging::Info(XORSTR("Initialized Fidget Spinner"));
#endif
    hacks::shared::spam::Init();
    backpacktf::init();
    logging::Info(XORSTR("Initialized Backpack.TF integration"));
#endif
#if not LAGBOT_MODE
    hacks::shared::walkbot::Initialize();
#endif
#if ENABLE_VISUALS
    hacks::shared::esp::Init();
#endif
    logging::Info(XORSTR("Clearing initializer stack"));
    while (!init_stack().empty())
    {
        init_stack().top()();
        init_stack().pop();
    }
    logging::Info(XORSTR("Initializer stack done"));

#if not ENABLE_VISUALS
    hack::command_stack().push(XORSTR("exec cat_autoexec_textmode"));
#endif
    hack::command_stack().push(XORSTR("exec cat_autoexec"));
    hack::command_stack().push(XORSTR("cat_killsay_reload"));
    hack::command_stack().push(XORSTR("cat_spam_reload"));
    hack::initialized = true;
}

void hack::Think()
{
    usleep(250000);
}

void hack::Shutdown()
{
    if (hack::shutdown)
        return;
    hack::shutdown = true;
    playerlist::Save();
#if ENABLE_VISUALS
    sdl_hooks::cleanSdlHooks();
#endif
    logging::Info(XORSTR("Unregistering convars.."));
    ConVar_Unregister();
#if not LAGBOT_MODE
    logging::Info(XORSTR("Shutting down killsay..."));
    hacks::shared::killsay::Shutdown();
    hacks::shared::announcer::shutdown();
#endif
    logging::Info(XORSTR("Success.."));
}
