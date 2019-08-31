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

#include <hacks/hacklist.hpp>
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
    unsigned int baseaddr = (unsigned int) info.dli_fbase - 1;

    for (auto i : st::stacktrace())
    {
        unsigned int offset = (unsigned int) (i.address()) - baseaddr;
        Dl_info info2;
        out << (void *) offset;
        if (!dladdr(i.address(), &info2))
        {
            out << std::endl;
            continue;
        }
        auto file_name = getFileName(info2.dli_fname);
        std::string path;
        if (sharedobj::LocateSharedObject(file_name, path))
        {
            link_map *lmap = nullptr;
            if ((lmap = static_cast<link_map *>(dlopen(path.c_str(), RTLD_NOLOAD))))
                out << " " << (void *) ((unsigned int) i.address() - lmap->l_addr - 1);
        }
        out << " " << info2.dli_fname << ": ";
        if (info2.dli_sname)
        {
            int status     = -4;
            char *realname = abi::__cxa_demangle(info2.dli_sname, nullptr, nullptr, &status);
            if (status == 0)
            {
                out << realname << std::endl;
                free(realname);
            }
            else
                out << info2.dli_sname << std::endl;
        }
        else
            out << "No Symbol" << std ::endl;
    }

    out.close();
    ::raise(SIGABRT);
}
#endif

static bool blacklist_file(const char *&filename)
{
    const static char *blacklist[] = { ".ani", ".wav", ".mp3", ".vvd", ".vtx", ".vtf", ".vfe", ".cache" /*, ".pcf"*/ };
    if (!filename || !std::strncmp(filename, "materials/console/", 18))
        return false;

    std::size_t len = std::strlen(filename);
    if (len <= 3)
        return false;

    auto ext_p = filename + len - 4;
    if (!std::strcmp(ext_p, ".vmt"))
    {
        /* Not loading it causes extreme console spam */
        if (std::strstr(filename, "corner"))
            return false;
        /* minor console spam */
        if (std::strstr(filename, "hud") || std::strstr(filename, "vgui"))
            return false;

        return true;
    }
    if (!std::strcmp(ext_p, ".mdl"))
    {
        return false;
    }
    if (!std::strncmp(filename, "/decal", 6))
        return true;

    for (int i = 0; i < sizeof(blacklist) / sizeof(blacklist[0]); ++i)
        if (!std::strcmp(ext_p, blacklist[i]))
            return true;

    return false;
}

static void *(*FSorig_Open)(void *, const char *, const char *, const char *);
static void *FSHook_Open(void *this_, const char *pFileName, const char *pOptions, const char *pathID)
{
    // fprintf(stderr, "Open: %s\n", pFileName);
    if (blacklist_file(pFileName))
        return nullptr;

    return FSorig_Open(this_, pFileName, pOptions, pathID);
}

static bool (*FSorig_ReadFile)(void *, const char *, const char *, void *, int, int, void *);
static bool FSHook_ReadFile(void *this_, const char *pFileName, const char *pPath, void *buf, int nMaxBytes, int nStartingByte, void *pfnAlloc)
{
    // fprintf(stderr, "ReadFile: %s\n", pFileName);
    if (blacklist_file(pFileName))
        return false;

    return FSorig_ReadFile(this_, pFileName, pPath, buf, nMaxBytes, nStartingByte, pfnAlloc);
}

static void *(*FSorig_OpenEx)(void *, const char *, const char *, unsigned, const char *, char **);
static void *FSHook_OpenEx(void *this_, const char *pFileName, const char *pOptions, unsigned flags, const char *pathID, char **ppszResolvedFilename)
{
    // fprintf(stderr, "OpenEx: %s\n", pFileName);
    if (pFileName && blacklist_file(pFileName))
        return nullptr;

    return FSorig_OpenEx(this_, pFileName, pOptions, flags, pathID, ppszResolvedFilename);
}

static int (*FSorig_ReadFileEx)(void *, const char *, const char *, void **, bool, bool, int, int, void *);
static int FSHook_ReadFileEx(void *this_, const char *pFileName, const char *pPath, void **ppBuf, bool bNullTerminate, bool bOptimalAlloc, int nMaxBytes, int nStartingByte, void *pfnAlloc)
{
    // fprintf(stderr, "ReadFileEx: %s\n", pFileName);
    if (blacklist_file(pFileName))
        return 0;

    return FSorig_ReadFileEx(this_, pFileName, pPath, ppBuf, bNullTerminate, bOptimalAlloc, nMaxBytes, nStartingByte, pfnAlloc);
}

static void (*FSorig_AddFilesToFileCache)(void *, void *, const char **, int, const char *);
static void FSHook_AddFilesToFileCache(void *this_, void *cacheId, const char **ppFileNames, int nFileNames, const char *pPathID)
{
    int i, j;

    fprintf(stderr, "AddFilesToFileCache: %d\n", nFileNames);
    for (i = 0; i < nFileNames; ++i)
        fprintf(stderr, "%s\n", ppFileNames[i]);
}

static int (*FSorig_AsyncReadMultiple)(void *, const char **, int, void *);
static int FSHook_AsyncReadMultiple(void *this_, const char **pRequests, int nRequests, void *phControls)
{
    for (int i = 0; pRequests && i < nRequests; ++i)
    {
        // fprintf(stderr, "AsyncReadMultiple %d %s\n", nRequests, pRequests[i]);
        if (blacklist_file(pRequests[i]))
        {
            if (nRequests > 1)
                fprintf(stderr, "FIXME: blocked AsyncReadMultiple for %d requests due to some filename being blacklisted\n", nRequests);
            /* FSASYNC_ERR_FILEOPEN */
            return -1;
        }
    }
    return FSorig_AsyncReadMultiple(this_, pRequests, nRequests, phControls);
}

static const char *(*FSorig_FindNext)(void *, void *);
static const char *FSHook_FindNext(void *this_, void *handle)
{
    const char *p;
    do
        p = FSorig_FindNext(this_, handle);
    while (p && blacklist_file(p));

    return p;
}

static const char *(*FSorig_FindFirst)(void *, const char *, void **);
static const char *FSHook_FindFirst(void *this_, const char *pWildCard, void **pHandle)
{
    auto p = FSorig_FindFirst(this_, pWildCard, pHandle);
    while (p && blacklist_file(p))
        p = FSorig_FindNext(this_, *pHandle);

    return p;
}

static bool (*FSorig_Precache)(const char *, const char *);
static bool FSHook_Precache(const char *pFileName, const char *pPathID)
{
    return true;
}

static CatCommand debug_invalidate("invalidate_mdl_cache", "Invalidates MDL cache", []() { g_IBaseClient->InvalidateMdlCache(); });

static hooks::VMTHook fs_hook{}, fs_hook2{};

static void ReduceRamUsage()
{
    if (fs_hook.IsHooked(reinterpret_cast<void *>(g_IFileSystem)))
        return;
    /* TO DO: Improves load speeds but doesn't reduce memory usage a lot
     * It seems engine still allocates significant parts without them
     * being really used
     * Plan B: null subsystems (Particle, Material, Model partially, Sound and etc.)
     */

    fs_hook.Set(reinterpret_cast<void *>(g_IFileSystem));
    fs_hook.HookMethod(FSHook_FindFirst, 27, &FSorig_FindFirst);
    fs_hook.HookMethod(FSHook_FindNext, 28, &FSorig_FindNext);
    fs_hook.HookMethod(FSHook_AsyncReadMultiple, 37, &FSorig_AsyncReadMultiple);
    fs_hook.HookMethod(FSHook_OpenEx, 69, &FSorig_OpenEx);
    fs_hook.HookMethod(FSHook_ReadFileEx, 71, &FSorig_ReadFileEx);
    fs_hook.HookMethod(FSHook_AddFilesToFileCache, 103, &FSorig_AddFilesToFileCache);
    fs_hook.Apply();

    fs_hook2.Set(reinterpret_cast<void *>(g_IFileSystem), 4);
    fs_hook2.HookMethod(FSHook_Open, 2, &FSorig_Open);
    fs_hook2.HookMethod(FSHook_Precache, 9, &FSorig_Precache);
    fs_hook2.HookMethod(FSHook_ReadFile, 14, &FSorig_ReadFile);
    fs_hook2.Apply();
    /* Might give performance benefit, but mostly fixes annoying console
     * spam related to mdl not being able to play sequence that it
     * cannot play on error.mdl
     */
    if (g_IBaseClient)
    {
        static BytePatch playSequence{ gSignatures.GetClientSignature, "55 89 E5 57 56 53 83 EC ? 8B 75 0C 8B 5D 08 85 F6 74 ? 83 BB", 0x00, { 0xC3 } };
        playSequence.Patch();
    }
#if 0
    /* Same explanation as above, but spams about certain particles not loaded */
    static BytePatch particleCreate{ gSignatures.GetClientSignature,
        "55 89 E5 56 53 83 EC ? 8B 5D 0C 8B 75 08 85 DB 74 ? A1",
        0x00, { 0x31, 0xC0, 0xC3 }
    };
    static BytePatch particlePrecache{ gSignatures.GetClientSignature,
        "55 89 E5 53 83 EC ? 8B 5D 0C 8B 45 08 85 DB 74 ? 80 3B 00 75 ? 83 C4 ? 5B 5D C3 ? ? ? ? 89 5C 24",
        0x00, { 0x31, 0xC0, 0xC3 }
    };
    static BytePatch particleCreating{ gSignatures.GetClientSignature,
        "55 89 E5 57 56 53 83 EC ? A1 ? ? ? ? 8B 75 08 85 C0 74 ? 8B 4D",
        0x00, { 0x31, 0xC0, 0xC3 }
    };
    particleCreate.Patch();
    particlePrecache.Patch();
    particleCreating.Patch();
#endif
}

static void UnHookFs()
{
    fs_hook.Release();
    fs_hook2.Release();
    if (g_IBaseClient)
        g_IBaseClient->InvalidateMdlCache();
}
#if !ENABLE_VISUALS
static InitRoutineEarly nullify_textmode([]() {
    ReduceRamUsage();
    static auto addr1 = e8call_direct(gSignatures.GetEngineSignature("E8 ? ? ? ? 8B 93 ? ? ? ? 85 D2 0F 84 ? ? ? ?")) + 0x18;
    static auto addr2 = sharedobj::materialsystem().Pointer(0x3EC08);

    static BytePatch patch1(addr1, { 0x81, 0xC4, 0x6C, 0x20, 0x00, 0x00, 0x5B, 0x5E, 0x5F, 0x5D, 0xC3 });
    static BytePatch patch2(addr2, { 0x83, 0xC4, 0x50, 0x5B, 0x5E, 0x5D, 0xC3 });

    patch1.Patch();
    patch2.Patch();
});
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
    InitRandom();
    sharedobj::LoadEarlyObjects();
    CreateEarlyInterfaces();
    logging::Info("Clearing Early initializer stack");
    null_graphics.installChangeCallback([](settings::VariableBase<bool> &, bool after) {
        if (after)
            ReduceRamUsage();
        else
            UnHookFs();
    });
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

    hooks::panel.Set(g_IPanel);
    hooks::panel.HookMethod(hooked_methods::methods::PaintTraverse, offsets::PaintTraverse(), &hooked_methods::original::PaintTraverse);
    hooks::panel.Apply();

#if ENABLE_VISUALS
    hooks::vstd.Set((void *) g_pUniformStream);
    hooks::vstd.HookMethod(HOOK_ARGS(RandomInt));
    hooks::vstd.Apply();

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
    hooks::input.Apply();

    hooks::modelrender.Set(g_IVModelRender);
    hooks::modelrender.HookMethod(HOOK_ARGS(DrawModelExecute));
    hooks::modelrender.Apply();

    hooks::enginevgui.Set(g_IEngineVGui);
    hooks::enginevgui.HookMethod(HOOK_ARGS(Paint));
    hooks::enginevgui.Apply();

    hooks::engine.Set(g_IEngine);
    hooks::engine.HookMethod(HOOK_ARGS(ServerCmdKeyValues));
    hooks::engine.Apply();

    hooks::eventmanager2.Set(g_IEventManager2);
    hooks::eventmanager2.HookMethod(HOOK_ARGS(FireEvent));
    hooks::eventmanager2.HookMethod(HOOK_ARGS(FireEventClientSide));
    hooks::eventmanager2.Apply();

    hooks::steamfriends.Set(g_ISteamFriends);
    hooks::steamfriends.HookMethod(HOOK_ARGS(GetFriendPersonaName));
    hooks::steamfriends.Apply();

    // FIXME [MP]
    logging::Info("Hooked!");
    velocity::Init();
    playerlist::Load();

#if ENABLE_VISUALS

    InitStrings();
#ifndef FEATURE_EFFECTS_DISABLED
    if (g_ppScreenSpaceRegistrationHead && g_pScreenSpaceEffects)
    {
        effect_chams::g_pEffectChams = new CScreenSpaceEffectRegistration("_cathook_chams", &effect_chams::g_EffectChams);
        g_pScreenSpaceEffects->EnableScreenSpaceEffect("_cathook_chams");
        effect_glow::g_pEffectGlow = new CScreenSpaceEffectRegistration("_cathook_glow", &effect_glow::g_EffectGlow);
        g_pScreenSpaceEffects->EnableScreenSpaceEffect("_cathook_glow");
    }
    logging::Info("SSE enabled..");
#endif
    sdl_hooks::applySdlHooks();
    logging::Info("SDL hooking done");

#endif /* TEXTMODE */
#if ENABLE_VISUALS
#ifndef FEATURE_FIDGET_SPINNER_ENABLED
    InitSpinner();
    logging::Info("Initialized Fidget Spinner");
#endif
#endif
    logging::Info("Clearing initializer stack");
    while (!init_stack().empty())
    {
        init_stack().top()();
        init_stack().pop();
    }
    logging::Info("Initializer stack done");
#if not ENABLE_VISUALS
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
