/*
 * Created by BenCat07 on
 * 11th of May 2020
 *
 */
#pragma once

#include <cstdint>
#include <memory>

#include "bytepatch.hpp"

#define foffset(p, i) ((unsigned char *) &p)[i]

class DetourHook
{
    uintptr_t original_address;
    std::unique_ptr<BytePatch> patch;
    void *hook_fn;

public:
    DetourHook()
    {
    }

    DetourHook(uintptr_t original, void *hook_func)
    {
        Init(original, hook_func);
    }

    void Init(uintptr_t original, void *hook_func)
    {
        original_address   = original;
        hook_fn            = hook_func;
        uintptr_t rel_addr = ((uintptr_t) hook_func - ((uintptr_t) original_address)) - 5;
        patch.reset(new BytePatch(original, { 0xE9, foffset(rel_addr, 0), foffset(rel_addr, 1), foffset(rel_addr, 2), foffset(rel_addr, 3) }));
        InitBytepatch();
    }

    void InitBytepatch()
    {
        if (patch)
            (*patch).Patch();
    }

    void Shutdown()
    {
        if (patch)
            (*patch).Shutdown();
    }

    ~DetourHook()
    {
        Shutdown();
    }

    // Gets the original function with no patches
    void *GetOriginalFunc() const
    {
        if (patch)
        {
            // Unpatch
            (*patch).Shutdown();
            return (void *) original_address;
        }
        // No patch, no func.
        return nullptr;
    }

    // Restore the patches
    inline void RestorePatch()
    {
        InitBytepatch();
    }
};
