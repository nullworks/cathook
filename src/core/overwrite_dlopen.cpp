#include "bytepatch.hpp"

// This is specifically for preload, and removes the source lock from the launcher. The only other way is bytepatching hl2_linux directly...
typedef void *(*dlopen_t)(const char *__file, int __mode);
void *dlopen(const char *__file, int __mode) noexcept(true)
{
    dlopen_t dlopen_fn = (dlopen_t) dlsym(RTLD_NEXT, "dlopen");
    auto ret           = dlopen_fn(__file, __mode);
    if (!strcmp(__file, "bin/launcher.so"))
    {
        logging::Info("Intercepted launcher.so");
        logging::Info("Waiting for cathook to load Launcher symbols...");
        while (sharedobj::launcher().lmap == nullptr)
            usleep(10);
        logging::Info("Loaded Launcher symbols");
        static uintptr_t launcher_sig      = gSignatures.GetLauncherSignature("55 89 E5 56 53 8D 9D ? ? ? ? 81 EC A0 00 00 00");
        static BytePatch LauncherBytePatch = BytePatch(launcher_sig, { 0xB8, 0x01, 0x00, 0x00, 0x00, 0xC3 });
        LauncherBytePatch.Patch();
        logging::Info("Removed source lock");
    }
    return ret;
}
