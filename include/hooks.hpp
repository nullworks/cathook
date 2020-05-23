/*
 * hooks.h
 *
 *  Created on: Oct 4, 2016
 *      Author: nullifiedcat
 */

#pragma once

// Parts of copypasted code
// Credits: Casual_Hacker

#include <stdint.h>
#include <stddef.h>

namespace hooks
{

typedef void *ptr_t;
typedef void *method_t;
typedef method_t *method_table_t;
typedef method_table_t *table_ptr_t;
typedef method_table_t &table_ref_t;

constexpr size_t ptr_size = sizeof(ptr_t);

unsigned CountMethods(method_table_t table);
void ReleaseAllHooks();
table_ref_t GetVMT(ptr_t inst, uint32_t offset = 0);

class VMTHook
{
public:
    VMTHook();
    ~VMTHook();
    void Set(ptr_t inst, uint32_t offset = 0);
    void Release();
    template <typename T> inline void HookMethod(T func, uint32_t idx, T *backup)
    {
        HookMethod(ptr_t(func), idx, (ptr_t *) (backup));
    }
    void HookMethod(ptr_t func, uint32_t idx, ptr_t *backup);
    void *GetMethod(uint32_t idx) const;
    void Apply();
    bool IsHooked(ptr_t inst);

public:
    ptr_t object{ nullptr };
    table_ptr_t vtable_ptr{ nullptr };
    method_table_t vtable_original{ nullptr };
    method_table_t vtable_hooked{ nullptr };
};

extern VMTHook panel;
extern VMTHook localbaseent;
extern VMTHook clientmode;
extern VMTHook clientmode4;
extern VMTHook client;
extern VMTHook prediction;
extern VMTHook inventory;
extern VMTHook chathud;
extern VMTHook engine;
extern VMTHook demoplayer;
extern VMTHook netchannel;
extern VMTHook firebullets;
extern VMTHook clientdll;
extern VMTHook matsurface;
extern VMTHook studiorender;
extern VMTHook input;
extern VMTHook modelrender;
extern VMTHook baseclientstate;
extern VMTHook baseclientstate8;
extern VMTHook steamfriends;
extern VMTHook soundclient;
extern VMTHook materialsystem;
extern VMTHook enginevgui;
extern VMTHook vstd;
extern VMTHook eventmanager2;
extern VMTHook cmdclientunrestricted;
} // namespace hooks
