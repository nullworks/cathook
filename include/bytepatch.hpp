#pragma once
#include <functional>
#include <stdio.h>
#include <string.h>
#include "core/logging.hpp"
#include <sys/mman.h>

class BytePatch
{
    void *addr{ 0 };
    size_t size;
    std::vector<unsigned char> patch_bytes;
    std::vector<unsigned char> original;
    bool patched{ false };

    std::function<uintptr_t(const char *)> SigScanFunc;
    const char *pattern{ nullptr };
    size_t offset{ 0 };

public:
    ~BytePatch()
    {
        Shutdown();
    }
    BytePatch(std::function<uintptr_t(const char *)> SigScanFunc, const char *pattern, size_t offset, std::vector<unsigned char> patch) : patch_bytes{ patch }, SigScanFunc(SigScanFunc), pattern(pattern), offset(offset)
    {
    }
    BytePatch(uintptr_t addr, std::vector<unsigned char> patch) : addr{ reinterpret_cast<void *>(addr) }, patch_bytes{ patch }
    {
    }
    BytePatch(void *addr, std::vector<unsigned char> patch) : addr{ addr }, patch_bytes{ patch }
    {
    }

    void Patch()
    {
        if (!patched)
        {
            if (!addr && pattern)
            {
                addr = (void *) (SigScanFunc)(pattern);
                if (!addr)
                {
                    logging::Info("Signature not found");
                    throw std::runtime_error("Signature not found");
                }
                addr = static_cast<void *>(static_cast<char *>(addr) + offset);
            }
            // Init her to allow Empty Initializors
            size = patch_bytes.size();
            original.resize(size);
            memcpy(&original[0], addr, size);

            void *page          = (void *) ((uint64_t) addr & ~0xFFF);
            void *end_page      = (void *) (((uint64_t)(addr) + size) & ~0xFFF);
            uintptr_t mprot_len = (uint64_t) end_page - (uint64_t) page + 0xFFF;

            mprotect(page, mprot_len, PROT_READ | PROT_WRITE | PROT_EXEC);
            memcpy(addr, &patch_bytes[0], size);
            mprotect(page, mprot_len, PROT_EXEC);
            patched = true;
        }
    }
    void Shutdown()
    {
        if (patched)
        {
            void *page          = (void *) ((uint64_t) addr & ~0xFFF);
            void *end_page      = (void *) (((uint64_t)(addr) + size) & ~0xFFF);
            uintptr_t mprot_len = (uint64_t) end_page - (uint64_t) page + 0xFFF;

            mprotect(page, mprot_len, PROT_READ | PROT_WRITE | PROT_EXEC);
            memcpy(addr, &original[0], size);
            mprotect(page, mprot_len, PROT_EXEC);
            patched = false;
        }
    }
};
