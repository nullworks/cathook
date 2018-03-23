
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

#include <cstddef> // size_t
#include <memory> // std::unique_ptr

// Class used to hook functions in objects with virtual functions
class VMTHook {
public:
	~VMTHook(){this->Release();}
	void Set(void* inst); // For when you start hooking.
	void HookMethod(void* func, size_t idx); // Give it the function you are replacing with and the number position of the function to replace
	void Apply(){*this->vtable_ptr = this->vtable_replacement.get();}	// To finish your hook
	void Release(); // To release this hook
	// Change parameter to something longer if needed
	template<class T>
	inline T GetMethod(size_t idx) const { return (T)vtable_original[idx]; }

	void*** vtable_ptr = nullptr; 		 // location to the pointer in the object that contains its vtable
	void** vtable_original = nullptr; // location of the objects original vtable
	std::unique_ptr<void*[]> vtable_replacement; // location of our vtable replacement
};
