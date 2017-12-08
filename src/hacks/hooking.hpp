
/*
 * hooks.h
 *
 *  Created on: Oct 4, 2016
 *      Author: nullifiedcat
 *
 *	Parts of copypasted code
 *	Credits: Casual_Hacker
 *
 */

#pragma once

// Class used to hook functions in objects with virtual functions
class VMTHook {
public:
	~VMTHook();
	void Set(void* inst, size_t offset = 0); // For when you start hooking.
	void HookMethod(void* func, size_t idx); // Give it the function you are replacing with and the number of the functions to replace
	void Apply();	// To finish your hook
	void Release(); // To release this hook
	inline void* GetMethod(size_t idx) const { return vtable_original[idx]; }

	void*** vtable_ptr = nullptr; 		 // location to the pointer in the object that contains its vtable
	void** vtable_original = nullptr; // location of the objects original vtable
	void** vtable_hooked = nullptr;   // location of our vtable replacement
};
