
/*
 *
 *	This file is used for getting pointers from signatures.
 *		Note: This file was stolen from darkstorm and refitted for the purposes of hookcat.
 *			  All credits goto gir for the amazing work on darkstorm <3
 *				-Onee
 *
 */

#include "../util/logging.hpp" // logging is cool

#include "signature.hpp"

namespace hacks {

// Credits to LEARN_MORE

#define INRANGE(x,a,b)		(x >= a && x <= b)
#define getBits( x )		(INRANGE(x,'0','9') ? (x - '0') : ((x&(~0x20)) - 'A' + 0xa))
#define getByte( x )		(getBits(x[0]) << 4 | getBits(x[1]))

static uintptr_t findPattern(const uintptr_t rangeStart, const uintptr_t rangeEnd, const char* pattern) {
	auto pat = reinterpret_cast<const unsigned char*>(pattern);
	uintptr_t firstMatch = 0;
	for (uintptr_t pCur = rangeStart; pCur < rangeEnd; ++pCur) {
		if (*(uint8_t*)pat == (unsigned char)'\?' || *(uint8_t*)pCur == (uint8_t)getByte(pat)) {
			if (!firstMatch) {
				firstMatch = pCur;
			}
			pat += (*(uint16_t*)pat == *(uint16_t*)"\?\?" || *(uint8_t*)pat != (unsigned char)'\?') ? 3 : 2;
			if (!*pat) {
				return firstMatch;
			}
		} else if (firstMatch) {
			pCur = firstMatch;
			pat = reinterpret_cast<const unsigned char*>(pattern);
			firstMatch = 0;
		}
	}
	return (uintptr_t)nullptr;
}

// This is just a temp function to get info our of our shared object
uintptr_t GetObjectSignature(const SharedObject& shared_obj, const char* chPattern) {
	uintptr_t address = 0;
	size_t size = 0;
	// Find a nicer way to not do this every time we need a sig
	/*dl_iterate_phdr([] (struct dl_phdr_info* info, size_t, void*) {
			address = info->dlpi_addr + info->dlpi_phdr[0].p_vaddr;
			size = info->dlpi_phdr[0].p_memsz;
			return 0;
	}, nullptr);*/
	if (address == 0 || size == 0) {
		g_CatLogging.log("Couldnt find signature for \"%s\" at \"%s\"",shared_obj.file_name, chPattern);
		return 0;
	}
	return findPattern(address, size, chPattern);
}

}
