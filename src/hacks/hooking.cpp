
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

void VMTHook::Set(void* inst, size_t offset) {
	this->Release(); // Release any hooks currently in place

	// Get our vtable from out base pointer and offset
	this->vtable_ptr = reinterpret_cast<void***>((uintptr_t)inst + (uintptr_t)offset);
	this->vtable_original = *vtable_ptr; // save original for release

	// Count how many functions avaible
	auto func_count = -1;
	while (this->vtable_original[func_count + 1])
		func_count++;
	g_CatLogging.log("Hooking vtable 0x%08x with %i methods", this->vtable_ptr, func_count);

	// Create a copy of the current vtable
	this->vtable_replacement = &static_cast<void**>(calloc(func_count + 1, sizeof(void*)))[1]; // add 2 to the count and make the pointer 2 of the alloc to give room for the guards
	memcpy(this->vtable_replacement, *this->vtable_ptr, sizeof(void*) * func_count);

	// Set hook information into the hook vtable
	this->vtable_replacement[-1] = GUARD;
}

void VMTHook::HookMethod(void* func, size_t idx) {
	g_CatLogging.log("Hooking method %d of vtable 0x%08x, replacing 0x%08x", idx, this->vtable_ptr, this->GetMethod(idx));
	this->vtable_replacement[idx] = func;
}

void VMTHook::Apply() {
	// Set the vtable pointer in the object to point to our new vtable
	*this->vtable_ptr = this->vtable_replacement;
}

VMTHook::~VMTHook() { Release(); }

void VMTHook::Release() {
	// Check if we have a vtable pointer and that the vtable pointer points to our replacement
	if (!this->vtable_ptr || *this->vtable_ptr != this->vtable_replacement)
		return;
	g_CatLogging.log("Un-hooking 0x%08x (vtable @ 0x%08x)", vtable_ptr, *vtable_ptr);
	// Check for our guard, then set the objects vtable pointer back to the original
	if (*this->vtable_ptr[-1] == GUARD)
		*this->vtable_ptr = this->vtable_original;
	// Free our vtables memory that we allocated earlier
	free(this->vtable_replacement[-1]);
}
