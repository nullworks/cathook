/*
 * hack.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#define __USE_GNU
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <boost/stacktrace.hpp>
#include <cxxabi.h>
#include <visual/SDLHooks.hpp>
#include "hack.hpp"
#include "common.hpp"
#include "MiscTemporary.hpp"
#if ENABLE_GUI
#include "menu/GuiInterface.hpp"
#endif
#include <link.h>
#include <pwd.h>

#include <hacks/hacklist.hpp>
#include "teamroundtimer.hpp"
#if EXTERNAL_DRAWING
#include "xoverlay.h"
#endif
#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

#include "copypasted/CDumper.hpp"
#include "version.h"
#include <cxxabi.h>

/*
 *  Credits to josh33901 aka F1ssi0N for butifel F1Public and Darkstorm 2015
 * Linux
 */

// game_shutdown = Is full game shutdown or just detach
bool hack::game_shutdown = true;
bool hack::shutdown      = false;
bool hack::initialized   = false;

const std::string &hack::GetVersion()
{
    static std::string version("Unknown Version");
    static bool version_set = false;
    if (version_set)
        return version;
#if defined(GIT_COMMIT_HASH) && defined(GIT_COMMITTER_DATE)
    version = "Version: #" GIT_COMMIT_HASH " " GIT_COMMITTER_DATE;
#endif
    version_set = true;
    return version;
}

const std::string &hack::GetType()
{
    static std::string version("Unknown Type");
    static bool version_set = false;
    if (version_set)
        return version;
    version = "";
#if not ENABLE_IPC
    version += " NOIPC";
#endif
#if not ENABLE_GUI
    version += " NOGUI";
#else
    version += " GUI";
#endif

#ifndef DYNAMIC_CLASSES

#ifdef GAME_SPECIFIC
    version += " GAME " TO_STRING(GAME);
#else
    version += " UNIVERSAL";
#endif

#else
    version += " DYNAMIC";
#endif

#if not ENABLE_VISUALS
    version += " NOVISUALS";
#endif

    version     = version.substr(1);
    version_set = true;
    return version;
}

std::mutex hack::command_stack_mutex;
std::stack<std::string> &hack::command_stack()
{
    static std::stack<std::string> stack;
    return stack;
}

void hack::ExecuteCommand(const std::string &command)
{
    std::lock_guard<std::mutex> guard(hack::command_stack_mutex);
    hack::command_stack().push(command);
}

#if ENABLE_LOGGING

std::string getFileName(std::string filePath)
{
    // Get last dot position
    std::size_t dotPos = filePath.rfind('.');
    std::size_t sepPos = filePath.rfind('/');

    if (sepPos != std::string::npos)
    {
        return filePath.substr(sepPos + 1, filePath.size() - (dotPos != std::string::npos ? 1 : dotPos));
    }
    return filePath;
}

void critical_error_handler(int signum)
{
    namespace st = boost::stacktrace;
    ::signal(signum, SIG_DFL);
    passwd *pwd = getpwuid(getuid());
    std::ofstream out(strfmt("/tmp/cathook-%s-%d-segfault.log", pwd->pw_name, getpid()).get());

    Dl_info info;
    if (!dladdr(reinterpret_cast<void *>(hack::ExecuteCommand), &info))
        return;

    for (auto i : st::stacktrace())
    {
        Dl_info info2;
        if (dladdr(i.address(), &info2))
        {
            unsigned int offset = (unsigned int) i.address() - (unsigned int) info2.dli_fbase;
            out << (!strcmp(info2.dli_fname, info.dli_fname) ? "cathook" : info2.dli_fname) << '\t' << (void *) offset << std::endl;
        }
    }

    out.close();
    ::raise(SIGABRT);
}
#endif

static void InitRandom()
{
    int rand_seed;
    FILE *rnd = fopen("/dev/urandom", "rb");
    if (!rnd || fread(&rand_seed, sizeof(rand_seed), 1, rnd) < 1)
    {
        logging::Info("Warning!!! Failed read from /dev/urandom (%s). Randomness is going to be weak", strerror(errno));
        timespec t;
        clock_gettime(CLOCK_MONOTONIC, &t);
        rand_seed = t.tv_nsec ^ t.tv_sec & getpid();
    }
    srand(rand_seed);
    if (rnd)
        fclose(rnd);
}

void hack::Hook()
{
    uintptr_t *clientMode = 0;
    // Bad way to get clientmode.
    // FIXME [MP]?
    while (!(clientMode = **(uintptr_t ***) ((uintptr_t)((*(void ***) g_IBaseClient)[10]) + 1)))
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

#if ENABLE_VISUALS || ENABLE_TEXTMODE
    hooks::panel.Set(g_IPanel);
    hooks::panel.HookMethod(hooked_methods::methods::PaintTraverse, offsets::PaintTraverse(), &hooked_methods::original::PaintTraverse);
    hooks::panel.Apply();
#endif

    hooks::vstd.Set((void *) g_pUniformStream);
    hooks::vstd.HookMethod(HOOK_ARGS(RandomInt));
    hooks::vstd.Apply();
#if ENABLE_VISUALS

    auto chat_hud = g_CHUD->FindElement("CHudChat");
    while (!(chat_hud = g_CHUD->FindElement("CHudChat")))
        usleep(1000);
    hooks::chathud.Set(chat_hud);
    hooks::chathud.HookMethod(HOOK_ARGS(StartMessageMode));
    hooks::chathud.HookMethod(HOOK_ARGS(StopMessageMode));
    hooks::chathud.HookMethod(HOOK_ARGS(ChatPrintf));
    hooks::chathud.Apply();
#endif

    hooks::input.Set(g_IInput);
    hooks::input.HookMethod(HOOK_ARGS(GetUserCmd));
    hooks::input.HookMethod(HOOK_ARGS(CreateMoveInput));
    hooks::input.Apply();

#if ENABLE_VISUALS || ENABLE_TEXTMODE
    hooks::modelrender.Set(g_IVModelRender);
    hooks::modelrender.HookMethod(HOOK_ARGS(DrawModelExecute));
    hooks::modelrender.Apply();
#endif
    hooks::enginevgui.Set(g_IEngineVGui);
    hooks::enginevgui.HookMethod(HOOK_ARGS(Paint));
    hooks::enginevgui.Apply();

    hooks::engine.Set(g_IEngine);
    hooks::engine.HookMethod(HOOK_ARGS(ServerCmdKeyValues));
    hooks::engine.Apply();

    hooks::demoplayer.Set(demoplayer);
    hooks::demoplayer.HookMethod(HOOK_ARGS(IsPlayingTimeDemo));
    hooks::demoplayer.Apply();

    hooks::eventmanager2.Set(g_IEventManager2);
    hooks::eventmanager2.HookMethod(HOOK_ARGS(FireEvent));
    hooks::eventmanager2.HookMethod(HOOK_ARGS(FireEventClientSide));
    hooks::eventmanager2.Apply();

    hooks::steamfriends.Set(g_ISteamFriends);
    hooks::steamfriends.HookMethod(HOOK_ARGS(GetFriendPersonaName));
    hooks::steamfriends.Apply();

    hooks::soundclient.Set(g_ISoundEngine);
    hooks::soundclient.HookMethod(HOOK_ARGS(EmitSound1));
    hooks::soundclient.HookMethod(HOOK_ARGS(EmitSound2));
    hooks::soundclient.HookMethod(HOOK_ARGS(EmitSound3));
    hooks::soundclient.Apply();

    hooks::prediction.Set(g_IPrediction);
    hooks::prediction.HookMethod(HOOK_ARGS(RunCommand));
    hooks::prediction.Apply();

#if ENABLE_VISUALS
    sdl_hooks::applySdlHooks();
#endif

    // FIXME [MP]
    logging::Info("Hooked!");
}

void hack::Initialize()
{
#if ENABLE_LOGGING
    ::signal(SIGSEGV, &critical_error_handler);
    ::signal(SIGABRT, &critical_error_handler);
#endif
    time_injected = time(nullptr);
/*passwd *pwd   = getpwuid(getuid());
char *logname = strfmt("/tmp/cathook-game-stdout-%s-%u.log", pwd->pw_name,
time_injected);
freopen(logname, "w", stdout);
free(logname);
logname = strfmt("/tmp/cathook-game-stderr-%s-%u.log", pwd->pw_name,
time_injected);
freopen(logname, "w", stderr);
free(logname);*/
// Essential files must always exist, except when the game is running in text
// mode.
#if ENABLE_VISUALS

    {
        std::vector<std::string> essential = { "fonts/tf2build.ttf" };
        for (const auto &s : essential)
        {
            std::ifstream exists(paths::getDataPath("/" + s), std::ios::in);
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
    logging::Info("Initializing...");
    InitRandom();
    sharedobj::LoadEarlyObjects();

// Fix locale issues caused by steam update
#if ENABLE_TEXTMODE
    static BytePatch patch(gSignatures.GetEngineSignature, "74 ? 89 5C 24 ? 8D 9D ? ? ? ? 89 74 24", 0, { 0x71 });
    patch.Patch();
#endif

    CreateEarlyInterfaces();

    // Applying the defaults needs to be delayed, because preloaded Cathook can not properly convert SDL codes to names before TF2 init
    settings::Manager::instance().applyDefaults();

    logging::Info("Clearing Early initializer stack");
    while (!init_stack_early().empty())
    {
        init_stack_early().top()();
        init_stack_early().pop();
    }
    logging::Info("Early Initializer stack done");
    sharedobj::LoadAllSharedObjects();
    CreateInterfaces();
    CDumper dumper;
    dumper.SaveDump();
    logging::Info("Is TF2? %d", IsTF2());
    logging::Info("Is TF2C? %d", IsTF2C());
    logging::Info("Is HL2DM? %d", IsHL2DM());
    logging::Info("Is CSS? %d", IsCSS());
    logging::Info("Is TF? %d", IsTF());
    InitClassTable();

    BeginConVars();
    g_Settings.Init();
    EndConVars();

#if ENABLE_VISUALS
    draw::Initialize();
#if ENABLE_GUI
// FIXME put gui here
#endif

#endif /* TEXTMODE */

    gNetvars.init();
    InitNetVars();
    g_pLocalPlayer    = new LocalPlayer();
    g_pPlayerResource = new TFPlayerResource();
    g_pTeamRoundTimer = new CTeamRoundTimer();

    velocity::Init();
    playerlist::Load();

#if ENABLE_VISUALS
    InitStrings();
#endif /* TEXTMODE */
    logging::Info("Clearing initializer stack");
    while (!init_stack().empty())
    {
        init_stack().top()();
        init_stack().pop();
    }
    logging::Info("Initializer stack done");
#if ENABLE_TEXTMODE
    hack::command_stack().push("exec cat_autoexec_textmode");
#else
    hack::command_stack().push("exec cat_autoexec");
#endif
    auto extra_exec = std::getenv("CH_EXEC");
    if (extra_exec)
        hack::command_stack().push(extra_exec);

    hack::initialized = true;
    for (int i = 0; i < 12; i++)
    {
        re::ITFMatchGroupDescription *desc = re::GetMatchGroupDescription(i);
        if (!desc || desc->m_iID > 9) // ID's over 9 are invalid
            continue;
        if (desc->m_bForceCompetitiveSettings)
        {
            desc->m_bForceCompetitiveSettings = false;
            logging::Info("Bypassed force competitive cvars!");
        }
    }
    hack::Hook();
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
    // Stop cathook stuff
    settings::cathook_disabled.store(true);
    playerlist::Save();
#if ENABLE_VISUALS
    sdl_hooks::cleanSdlHooks();
#endif
    logging::Info("Unregistering convars..");
    ConVar_Unregister();
    logging::Info("Unloading sharedobjects..");
    sharedobj::UnloadAllSharedObjects();
    logging::Info("Deleting global interfaces...");
    delete g_pLocalPlayer;
    delete g_pTeamRoundTimer;
    delete g_pPlayerResource;
#if ENABLE_GUI
    logging::Info("Shutting down GUI");
    gui::shutdown();
#endif
    if (!hack::game_shutdown)
    {
        logging::Info("Running shutdown handlers");
        EC::run(EC::Shutdown);
#if ENABLE_VISUALS
        g_pScreenSpaceEffects->DisableScreenSpaceEffect("_cathook_glow");
        g_pScreenSpaceEffects->DisableScreenSpaceEffect("_cathook_chams");
#if EXTERNAL_DRAWING
        xoverlay_destroy();
#endif
#endif
    }
    logging::Info("Releasing VMT hooks..");
    hooks::ReleaseAllHooks();
    logging::Info("Success..");
}
