
/*
 *
 *	This file is used for getting pointers from signatures.
 *		Note: This file was stolen from darkstorm and refitted for the purposes of hookcat.
 *			  All credits goto gir for the amazing work on darkstorm <3
 *				-Onee
 *
 */

#include <unistd.h> // uintptr_t def
#include <elf.h> // linux related thing about so files, needs more explanation
#include <fcntl.h> // open()
#include <sys/mman.h> // linux related tool to map a file into memory
#include <string.h> // char string utils
#include <unordered_map>

#include "../util/logging.hpp" // logging is cool

#include "signature.hpp"

// TODO, introduce support for 64 bit processes

namespace hacks {

// Not sure how entirely this works, google doesnt help so ill just leave it for now...
static Elf32_Shdr* getSectionHeader(const void* module, const char* sectionName) {

	Elf32_Ehdr* ehdr = (Elf32_Ehdr*)module;
	Elf32_Shdr* shdr = (Elf32_Shdr*)(uintptr_t)module + ehdr->e_shoff;

	int shNum = ehdr->e_shnum;
	Elf32_Shdr* strhdr = &shdr[ehdr->e_shstrndx];

	char* strtab = NULL;
	int strtabSize = 0;
	if (strhdr != NULL && strhdr->sh_type == 3) {
		strtab = (char*)(uintptr_t)module + strhdr->sh_offset;
		if (strtab == NULL) g_CatLogging.log("String table was NULL!");
		strtabSize = strhdr->sh_size;
	} else g_CatLogging.log("String table header was corrupted!");

	for(int i = 0; i < ehdr->e_shnum; i++) {
		Elf32_Shdr *hdr = &shdr[i];
		if(hdr && hdr->sh_name < strtabSize) if(!strcmp(strtab + hdr->sh_name, sectionName)) return hdr;
	}
	return 0;
}

// Logic to turn char into hex?
#define INRANGE(x,a,b)   (x >= a && x <= b)
#define getBits( x )   	 (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define getByte( x )   	 (getBits(x[0]) << 4 | getBits(x[1]))

static uintptr_t dwFindPattern(const uintptr_t& dwAddress, const uintptr_t& dwLength, const char* szPattern) {

	// Bookkeepers
	const char* pat = szPattern; // keeper to check how much of the string we've went through
	uintptr_t firstMatch = NULL;

	// Loop through our adress range and look for our signatrue
	for (uintptr_t pCur = dwAddress; pCur < dwLength; pCur++) {
		if (!*pat) return firstMatch; // Check to make sure our pattern is still a thing
		// Check if the patern matches what we currently have. If its a question mark or the patern matches our current byte, we continue.
		// I give up at this point because it works and i dont care
		if (*(uint8_t*)pat == '\?' || *(uint8_t*)pCur == getByte(pat)) {
			if (!firstMatch) firstMatch = pCur;
			if (!pat[2]) return firstMatch;
			if (*(uintptr_t*)pat == '\?\?' || *(uint8_t *)pat != '\?') pat += 3;
			else pat += 2;
		} else {
			pat = szPattern;
			firstMatch = 0;
		}
	}
	return NULL;
}

uintptr_t GetObjectSignature(const SharedObject& shared_obj, const char* chPattern) {

	// We do our modules im memory differently from darkstorm due to the need of modularity. Shouldnt hurt anything as long as this isnt called too much
	const void* module = nullptr;

	// Get our modules raw data into memory
	static std::unordered_map<std::string, void*> stored_modules;
	const auto& module_find = stored_modules.find(shared_obj.path);
	// Check if we dont have a module in memory, if so we do so here
	if (module_find == stored_modules.end()) {
		// Some form of syscall to put the file in memory
		int fd = open(shared_obj.path.c_str(), O_RDONLY);
		void* tmp = mmap(NULL, lseek(fd, 0, SEEK_END), PROT_READ, MAP_SHARED, fd, 0);
		// Insert this into the map for future use
		stored_modules.insert({shared_obj.path, tmp});
		module = tmp;
	} else
		module = module_find->second;

	// Get stuff
	Elf32_Shdr* textHeader = getSectionHeader(module, ".text");
	int textOffset = textHeader->sh_offset;
	int textSize = textHeader->sh_size;

	// we need to remap the address that we got from the pattern search from our mapped file to the actual memory
	// we do this by rebasing the address (subbing the mmapped one and replacing it with the dlopened one.
	return dwFindPattern((uintptr_t)module + textOffset, (uintptr_t)module + textOffset + textSize, chPattern) - (uintptr_t)module + shared_obj.lmap->l_addr;
}

}
