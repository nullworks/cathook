/*
 *	
 *	This file was stolen from darkstorm and refitted for the purposes of hookcat
 *				-Onee
 *
 *
 */

#include <unistd.h>
#include <elf.h>
#include <link.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <cstring>

#include "../../../logging.h"
#include "sharedobj.h"

#include "CSignature.h"

#define INRANGE(x,a,b)   (x >= a && x <= b) 
#define getBits( x )   	 (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define getByte( x )   	 (getBits(x[0]) << 4 | getBits(x[1]))
	
// module should be a pointer to the base of an elf32 module
// this is not the value returned by dlopen (which returns an opaque handle to the module)
// the best method to get this address is with fopen() and mmap()
// (check the GetClientSignature() and GetEngineSignature() for an example)
Elf32_Shdr *getSectionHeader(void *module, const char *sectionName)
{
	// we need to get the modules actual address from the handle
	
	Elf32_Ehdr *ehdr = (Elf32_Ehdr *)module;
	Elf32_Shdr *shdr = (Elf32_Shdr *)((unsigned)module + ehdr->e_shoff);
	int shNum = ehdr->e_shnum;

	Elf32_Shdr *strhdr = &shdr[ehdr->e_shstrndx];

	char *strtab = NULL;
	int strtabSize = 0;
	if(strhdr != NULL && strhdr->sh_type == 3)
	{
		strtab = (char *)((unsigned)module + strhdr->sh_offset);

		if(strtab == NULL)
			//Log::Fatal("String table was NULL!");
			logging::Info("String table was NULL!");
		strtabSize = strhdr->sh_size;
	}
	else
	{
		//Log::Fatal("String table header was corrupted!");
		logging::Info("String table header was corrupted!");
	}
	
	for(int i = 0; i < ehdr->e_shnum; i++)
	{
		Elf32_Shdr *hdr = &shdr[i];
		if(hdr && hdr->sh_name < strtabSize)
		{
			if(!strcmp(strtab + hdr->sh_name, sectionName))
				return hdr;
		}
	}
	return 0;
}

uintptr_t CSignature::dwFindPattern(uintptr_t dwAddress, uintptr_t dwLength, const char* szPattern)
{
	const char* pat = szPattern;
	uintptr_t firstMatch = NULL;
	for (uintptr_t pCur = dwAddress; pCur < dwLength; pCur++)
	{
		if (!*pat) return firstMatch;
		if (*(uint8_t *)pat == '\?' || *(uint8_t *)pCur == getByte(pat)) {
			if (!firstMatch) firstMatch = pCur;
			if (!pat[2]) return firstMatch;
			if (*(uintptr_t *)pat == '\?\?' || *(uint8_t *)pat != '\?') pat += 3;
			else pat += 2;
		}
		else {
			pat = szPattern;
			firstMatch = 0;
		}
	}
	return NULL;
}
//===================================================================================
void *CSignature::GetModuleHandleSafe( const char* pszModuleName )
{
	void *moduleHandle = NULL;

	do
	{
		moduleHandle = dlopen( pszModuleName, RTLD_NOW );
		usleep( 1 );
	}
	while(moduleHandle == NULL);

	return moduleHandle;
}
//===================================================================================
uintptr_t CSignature::GetClientSignature(char* chPattern)
{
	// we need to do this becuase (i assume that) under the hood, dlopen only loads up the sections that it needs
	// into memory, meaning that we cannot get the string table from the module.
	static int fd = open(sharedobj::client().path.c_str(), O_RDONLY);
	static void *module = mmap(NULL, lseek(fd, 0, SEEK_END), PROT_READ, MAP_SHARED, fd, 0);
	static link_map *moduleMap = sharedobj::client().lmap;

	//static void *module = (void *)moduleMap->l_addr;
	
	static Elf32_Shdr *textHeader = getSectionHeader(module, ".text");

	static int textOffset = textHeader->sh_offset;

	static int textSize = textHeader->sh_size;
	
	// we need to remap the address that we got from the pattern search from our mapped file to the actual memory
	// we do this by rebasing the address (subbing the mmapped one and replacing it with the dlopened one.
	return dwFindPattern(((uintptr_t)module) + textOffset, ((uintptr_t)module) + textOffset + textSize, chPattern) - (uintptr_t)(module) + moduleMap->l_addr;
}
//===================================================================================
uintptr_t CSignature::GetEngineSignature(char* chPattern)
{
	// we need to do this becuase (i assume that) under the hood, dlopen only loads up the sections that it needs
	// into memory, meaning that we cannot get the string table from the module.
	static int fd = open(sharedobj::engine().path.c_str(), O_RDONLY);
	static void *module = mmap(NULL, lseek(fd, 0, SEEK_END), PROT_READ, MAP_SHARED, fd, 0);
	static link_map *moduleMap = sharedobj::engine().lmap;

	//static void *module = (void *)moduleMap->l_addr;
	
	static Elf32_Shdr *textHeader = getSectionHeader(module, ".text");

	static int textOffset = textHeader->sh_offset;

	static int textSize = textHeader->sh_size;
	
	// we need to remap the address that we got from the pattern search from our mapped file to the actual memory
	// we do this by rebasing the address (subbing the mmapped one and adding the dlopened one.
	return dwFindPattern(((uintptr_t)module) + textOffset, ((uintptr_t)module) + textOffset + textSize, chPattern) - (uintptr_t)(module) + moduleMap->l_addr;
}

CSignature gSignatures;
