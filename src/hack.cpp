/*
 * hack.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The code below was obtained from:
// http://stackoverflow.com/questions/77005/how-to-generate-a-stacktrace-when-my-gcc-c-app-crashes/1925461#1925461
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef __USE_GNU
#define __USE_GNU
#endif

#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <unistd.h>
#include <visual/SDLHooks.hpp>
#include "hack.hpp"
#include "common.hpp"
#include "MiscTemporary.hpp"

#include <hacks/hacklist.hpp>

#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

#include "copypasted/CDumper.hpp"
#include "version.h"

/*
 *  Credits to josh33901 aka F1ssi0N for butifel F1Public and Darkstorm 2015
 * Linux
 */

bool hack::shutdown    = false;
bool hack::initialized = false;

const std::string &hack::GetVersion()
{
    static std::string version("Unknown Version");
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

void hack::ExecuteCommand(const std::string command)
{
    std::lock_guard<std::mutex> guard(hack::command_stack_mutex);
    hack::command_stack().push(command);
}

/* This structure mirrors the one found in /usr/include/asm/ucontext.h */
typedef struct _sig_ucontext
{
    unsigned long uc_flags;
    struct ucontext *uc_link;
    stack_t uc_stack;
    struct sigcontext uc_mcontext;
    sigset_t uc_sigmask;
} sig_ucontext_t;

void crit_err_hdlr(int sig_num, siginfo_t *info, void *ucontext)
{
    void *array[50];
    void *caller_address;
    char **messages;
    int size, i;
    sig_ucontext_t *uc;

    uc = (sig_ucontext_t *) ucontext;

/* Get the address at the time the signal was raised */
#if defined(__i386__)                              // gcc specific
    caller_address = (void *) uc->uc_mcontext.eip; // EIP: x86 specific
#elif defined(__x86_64__)                          // gcc specific
    caller_address = (void *) uc->uc_mcontext.rip; // RIP: x86_64 specific
#else
#error Unsupported architecture. // TODO: Add support for other arch.
#endif

    fprintf(stderr, "\n");
    FILE *backtraceFile;

    // In this example we write the stacktrace to a file. However, we can also
    // just fprintf to stderr (or do both).
    passwd *pwd   = getpwuid(getuid());
    backtraceFile = fopen(
        strfmt("/tmp/cathook-%s-%d-segfault.log", pwd->pw_name, getpid()).get(),
        "w");

    if (sig_num == SIGSEGV)
        fprintf(backtraceFile, "signal %d (%s), address is %p from %p\n",
                sig_num, strsignal(sig_num), info->si_addr,
                (void *) caller_address);
    else
        fprintf(backtraceFile, "signal %d (%s)\n", sig_num, strsignal(sig_num));

    size = backtrace(array, 50);
    /* overwrite sigaction with caller's address */
    array[1] = caller_address;
    messages = backtrace_symbols(array, size);
    /* skip first stack frame (points here) */
    for (i = 1; i < size && messages != NULL; ++i)
    {
        fprintf(backtraceFile, "[bt]: (%d) %s\n", i, messages[i]);
    }

    fclose(backtraceFile);
    free(messages);

    exit(EXIT_FAILURE);
}

void installSignal(int __sig)
{
    struct sigaction sigact;
    sigact.sa_sigaction = crit_err_hdlr;
    sigact.sa_flags     = SA_RESTART | SA_SIGINFO;
    if (sigaction(__sig, &sigact, (struct sigaction *) NULL) != 0)
    {
        fprintf(stderr, "error setting signal handler for %d (%s)\n", __sig,
                strsignal(__sig));
        exit(EXIT_FAILURE);
    }
}
void hack::Initialize()
{
    signal(SIGPIPE, SIG_IGN);
    installSignal(SIGSEGV);
    installSignal(SIGABRT);
    installSignal(SIGINT);
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
        std::vector<std::string> essential = { "menu.json",
                                               "fonts/tf2build.ttf" };
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

    logging::Info("Initializing...");
    srand(time(0));
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
        logging::Info("Graphics Nullified");
        logging::Info("The game will crash");
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
    hacks::shared::killsay::init();
    hacks::shared::announcer::init();
    hacks::tf2::killstreak::init();
#endif
    hacks::shared::catbot::init();
    logging::Info("Hooked!");
    velocity::Init();
    playerlist::Load();

#if ENABLE_VISUALS

    InitStrings();
#ifndef FEATURE_EFFECTS_DISABLED
    if (g_ppScreenSpaceRegistrationHead && g_pScreenSpaceEffects)
    {
        effect_chams::g_pEffectChams = new CScreenSpaceEffectRegistration(
            "_cathook_chams", &effect_chams::g_EffectChams);
        g_pScreenSpaceEffects->EnableScreenSpaceEffect("_cathook_chams");
        effect_chams::g_EffectChams.Init();
        effect_glow::g_pEffectGlow = new CScreenSpaceEffectRegistration(
            "_cathook_glow", &effect_glow::g_EffectGlow);
        g_pScreenSpaceEffects->EnableScreenSpaceEffect("_cathook_glow");
    }
    logging::Info("SSE enabled..");
#endif
    sdl_hooks::applySdlHooks();
    logging::Info("SDL hooking done");

#endif /* TEXTMODE */
#if not LAGBOT_MODE
    hacks::shared::anticheat::Init();
#endif
#if ENABLE_VISUALS
#ifndef FEATURE_FIDGET_SPINNER_ENABLED
    InitSpinner();
    logging::Info("Initialized Fidget Spinner");
#endif
    hacks::shared::spam::init();
    backpacktf::init();
    logging::Info("Initialized Backpack.TF integration");
#endif
#if not LAGBOT_MODE
    hacks::shared::walkbot::Initialize();
#endif
#if ENABLE_VISUALS
    hacks::shared::esp::Init();
#endif
#if not ENABLE_VISUALS
    hack::command_stack().push("exec cat_autoexec_textmode");
#endif
    hack::command_stack().push("exec cat_autoexec");
    hack::command_stack().push("cat_killsay_reload");
    hack::command_stack().push("cat_spam_reload");

    logging::Info("Clearing initializer stack");
    while (!init_stack().empty())
    {
        init_stack().top()();
        init_stack().pop();
    }
    logging::Info("Initializer stack done");

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
    logging::Info("Unregistering convars..");
    ConVar_Unregister();
#if not LAGBOT_MODE
    logging::Info("Shutting down killsay...");
    hacks::shared::killsay::shutdown();
    hacks::shared::announcer::shutdown();
#endif
    logging::Info("Success..");
}
