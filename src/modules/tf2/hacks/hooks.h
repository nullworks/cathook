/*
 * hooks.h
 *
 *  Created on: Oct 4, 2016
 *      Author: nullifiedcat
 */

#ifndef HOOKS_H_
#define HOOKS_H_

// Parts of copypasted code
// Credits: Casual_Hacker

#include <stdint.h>
#include <stddef.h>

namespace hooks {

typedef void*			ptr_t;
typedef void* 			method_t;
typedef method_t* 		method_table_t;
typedef method_table_t* table_ptr_t;
typedef method_table_t& table_ref_t;

constexpr size_t ptr_size = sizeof(ptr_t);

unsigned CountMethods(method_table_t table);
table_ref_t GetVMT(ptr_t inst, uint32_t offset = 0);
bool IsHooked(ptr_t inst, uint32_t offset = 0);

constexpr uint32_t GUARD = 0xD34DC477;

class VMTHook {
public:
	VMTHook();
	~VMTHook();
	void Set(ptr_t inst, uint32_t offset = 0);
	void Release();
	void HookMethod(ptr_t func, uint32_t idx);
	void* GetMethod(uint32_t idx) const;
	void Apply();
public:
	ptr_t object 					{ nullptr };
	table_ptr_t vtable_ptr 			{ nullptr };
	method_table_t vtable_original 	{ nullptr };
	method_table_t vtable_hooked 	{ nullptr };
};

extern VMTHook panel;
extern VMTHook clientmode;
extern VMTHook clientmode4;
extern VMTHook client;
extern VMTHook netchannel;
extern VMTHook clientdll;
extern VMTHook matsurface;
extern VMTHook studiorender;
extern VMTHook input;
extern VMTHook modelrender;
extern VMTHook baseclientstate;
extern VMTHook baseclientstate8;
extern VMTHook steamfriends;
extern VMTHook materialsystem;

}

#endif /* HOOKS_H_ */
