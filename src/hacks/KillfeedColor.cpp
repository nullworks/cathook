/*
 *  Credits to UNKN0WN
 */
#include "common.hpp"

#if !ENFORCE_STREAM_SAFETY
typedef void (*GetTeamColor_t)(Color *, void *, int, int);

static GetTeamColor_t GetTeamColor_fn;

// This is used to Combine player IDX and team into one int for later usage
int encodeTPI(void * /* g_PR this ptr */, int playerIDX)
{
    int teamNum = g_pPlayerResource->GetTeam(playerIDX);
    return (playerIDX << 12) | teamNum;
}

void GetTeamColor(Color *clr, void *this_, int data, int local)
{
    // Decode the data which was encoded above
    int playerIdx = data >> 12;
    int team      = data & 0x0FFF;
    player_info_s pinfo;

    // Our encoding uses quite a few bits, so let's just check for < 1000 and !playerIdx
    if (data < 1000 || !playerIdx)
        return GetTeamColor_fn(clr, this_, data, local);
    // No Color change
    if (!g_IEngine->GetPlayerInfo(playerIdx, &pinfo) || playerlist::IsDefault(pinfo.friendsID))
        return GetTeamColor_fn(clr, this_, team, local);

    // Get new color
    auto wc = playerlist::Color(pinfo.friendsID);

    // Local player needs slightly different coloring in killfeed
    if (local)
        for (int i = 0; i < 3; i++)
            wc[i] /= 1.125f;

    clr->SetColor(wc.r * 255, wc.g * 255, wc.b * 255, 255);
}

static std::vector<BytePatch> no_stack_smash{};
static std::vector<BytePatch> color_patches{};
// Defines to make our lives easier
#define DONT_SMASH_SMACK(offset) no_stack_smash.push_back(BytePatch{ addr, std::vector<unsigned char>{ 0xC3, 0x90, 0x90 } });
#define PATCH_COLORS(patchNum) color_patches.push_back(BytePatch{ addrs[patchNum], patches[patchNum] });

static InitRoutine init([] {
    uintptr_t addr = gSignatures.GetClientSignature("55 89 E5 53 8B 55 10 8B 45 08 8B 4D 0C 8B 5D 14");
    if (!addr)
        return;

    GetTeamColor_fn = GetTeamColor_t(addr);
    /*
    01143E7E
     */
    /* clang-format off */
    std::array<uintptr_t, 8> addrs = {
        gSignatures.GetClientSignature("8B 06 89 54 24 0C 8B 93"),                      // 26 bytes free
        gSignatures.GetClientSignature("8B 06 89 54 24 0C 8B 53 40"),                   // 23 bytes free
        gSignatures.GetClientSignature("FF 92 ? ? ? ? 89 47 40"),                       // 6 bytes free
        addrs[2] + 47,                                                                  // 6 bytes free
        gSignatures.GetClientSignature("FF 92 ? ? ? ? 8B 4D 10 89 43 40"),              // 6 bytes free
        addrs[4] + 72,                                                                  // 6 bytes free
        gSignatures.GetClientSignature("FF 92 ? ? ? ? 39 B5 ? ? ? ? 89 47 40 0F 85"),   // 6 bytes free
        gSignatures.GetClientSignature("FF 92 ? ? ? ? 39 B5 ? ? ? ? 89 47 40 0F 84")    // 6 bytes free
    };
    /* clang-format on */

    for (int i = 0; i < addrs.size(); i++) // null check
        if (!addrs[i])
            return;

    std::array<std::vector<unsigned char>, addrs.size()> patches;
    // 21 byte patch
    // 1st 8 bytes free
    // 2nd 5 bytes free (JUST ENOUGH)
    /*
mov [esp+12], edx
mov edx, [ebx+0x84]
mov [esp+8], edx
mov [esp+4], esi
mov [esp], ecx
 */
    patches[0] = patches[1] = { 0x89, 0x54, 0x24, 0x0C, 0x8B, 0x93, 0x84, 0x00, 0x00, 0x00, 0x89, 0x54, 0x24, 0x08, 0x89, 0x74, 0x24, 0x04, 0x89, 0x0C, 0x24, 0xE8 };
    for (int i = 2; i < addrs.size(); i++)
        patches[i] = { 0xE8 };

    for (int i = 0; i < addrs.size(); i++)
    {
        uintptr_t relAddr = ((i < 2 ? (uintptr_t) GetTeamColor : (uintptr_t) encodeTPI) - ((uintptr_t) addrs[i] + (i < 2 ? 21 : 0))) - 5;
        for (int j = 0; j < 4; j++)
            patches[i].push_back(((unsigned char *) &relAddr)[j]);
    }

    auto addNops = [](std::vector<unsigned char> &p, int totalSize) {
        for (int i = p.size(); i < totalSize; i++)
            p.push_back(0x90);
    };
    addNops(patches[0], 26 + 3); // 3 extra for removal of sub  esp, 4
    for (int i = 2; i < addrs.size(); i++)
        addNops(patches[i], 6);

    for (int i = 0; i < 2; i++)
    {
        patches[i].insert(patches[i].end(), { 0x8D, 0x85, 0xE8, 0xFB, 0xFF, 0xFF });
        if (i == 1)
            patches[i][6] = 0x40, patches[i][29] = 0xF7;
    }
    /* clang-format off */
    PATCH_COLORS(0); // GetTeamColor call patch victim
    PATCH_COLORS(1); // GetTeamColor call patch killer
    PATCH_COLORS(2); // GetTeam call patch victim
    PATCH_COLORS(3); // GetTeam call patch killer
    PATCH_COLORS(4); // GetTeam call patch domination killer
    PATCH_COLORS(5); // GetTeam call patch domination victim
    PATCH_COLORS(6); // GetTeam call patch capture/defend flag, defend bomb 1
    PATCH_COLORS(7); // GetTeam call patch (?) defend bomb 2

    // This fixes release mode crashes due to double stack cleanups
    DONT_SMASH_SMACK(68);
    DONT_SMASH_SMACK(90);
    DONT_SMASH_SMACK(121);
    DONT_SMASH_SMACK(138);
    DONT_SMASH_SMACK(154);

    for (auto &i : color_patches)
        i.Patch();
    for (auto &i : no_stack_smash)
        i.Patch();
    EC::Register(EC::Shutdown, [](){
        for (auto &i : color_patches)
            i.Shutdown();
        for (auto &i : no_stack_smash)
            i.Shutdown();
    }, "shutdown_kfc");
});

#endif
