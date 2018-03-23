
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

void VMTHook::Set(void* inst) {
	this->Release(); // Release any hooks currently in place

	// Get our vtable from out base pointer and offset
	this->vtable_ptr = (void***)inst;
	this->vtable_original = *vtable_ptr; // save original for release

	// Count how many functions avaible
	auto func_count = 0;
	while (this->vtable_original[func_count])
		func_count++;
	g_CatLogging.log("Hooking vtable 0x%08x with %i methods", this->vtable_ptr, func_count);

	// Create a copy of the current vtable
	this->vtable_replacement = std::make_unique<void*[]>(func_count);; // add 2 to the count and make the pointer 2 of the alloc to give room for the guards
	memcpy(this->vtable_replacement.get(), *this->vtable_ptr, sizeof(void*) * func_count);
}

void VMTHook::HookMethod(void* func, size_t idx) {
	g_CatLogging.log("Hooking method %d of vtable 0x%08x, replacing 0x%08x", idx, this->vtable_ptr, this->GetMethod<void*>(idx));
	this->vtable_replacement[idx] = func;
}

void VMTHook::Release() {
	// Check if we have a vtable pointer and that the vtable pointer points to our replacement
	if (!this->vtable_ptr || *this->vtable_ptr != this->vtable_replacement.get())
		return;
	g_CatLogging.log("Un-hooking 0x%08x (vtable @ 0x%08x)", vtable_ptr, *vtable_ptr);
	*this->vtable_ptr = this->vtable_original;
}


// TODO, asm hook
// This is for hooking non-virtual function
template<typename Replacement>
class ASMHook {
public:
	ASMHook();
};
