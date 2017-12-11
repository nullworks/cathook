
/*
 * hooks.cpp
 *
 *  Created on: Oct 4, 2016
 *      Author: nullifiedcat
 */

#include <stdlib.h> // calloc
#include <string.h> // memcpy

#include "../util/logging.hpp"

#include "hooking.hpp"

// Guard to tell when we have a hooked table
#define GUARD (void*)0xD34DC477 // :thonkfire:

static unsigned CountMethods(void** table) {
	unsigned int i = -1;
	do ++i; while (table[i]);
	return i;
}

void VMTHook::Set(void* inst, size_t offset) {
	// Release any hooks currently in place
	Release();
	g_CatLogging.log("release");
	// Get our vtable from out base pointer and offset
	vtable_ptr = reinterpret_cast<void***>((uintptr_t)inst + (uintptr_t)offset);
	vtable_original = *vtable_ptr; // save original for release
	g_CatLogging.log("Got vtableptr and orig");
	// Count how many functions aviable6
	int func_count = CountMethods(vtable_original);
	// Make a log of what we have so far
	g_CatLogging.log("Hooking vtable 0x%08x with %i methods", vtable_ptr, func_count);
	// Create a copy of the current vtable
	vtable_hooked = &static_cast<void**>(calloc(func_count + 2, sizeof(void*)))[2]; // add 2 to the count and make the pointer 2 of the alloc to give room for the guards
	memcpy(vtable_hooked, *vtable_ptr, sizeof(void*) * func_count);

	// Set hook information into the hook vtable
	vtable_hooked[-2] = this;
	vtable_hooked[-1] = GUARD;
}

void VMTHook::HookMethod(void* func, size_t idx) {
	g_CatLogging.log("Hooking method %d of vtable 0x%08x, replacing 0x%08x", idx, vtable_ptr, GetMethod(idx));
	vtable_hooked[idx] = func;
}

void VMTHook::Apply() {
	// Set the vtable pointer in the object to point to our new vtable
	*vtable_ptr = vtable_hooked;
}

VMTHook::~VMTHook() { Release(); }

void VMTHook::Release() {
	// Check if we have a vtable pointer and that the vtable pointer points to our replacement
	if (!(vtable_ptr && *vtable_ptr == vtable_hooked)) return;
	g_CatLogging.log("Un-hooking 0x%08x (vtable @ 0x%08x)", vtable_ptr, *vtable_ptr);
	// Check for our guard, then set the objects vtable pointer back to the original
	if (*vtable_ptr[-1] == GUARD) *vtable_ptr = vtable_original;
	// Free our vtables memory that we allocated earlier
	free(vtable_hooked[-2]);
}
