/*
 * CTFInventoryManager.cpp
 *
 *  Created on: Apr 26, 2018
 *      Author: bencat07
 */
#include "common.hpp"
using namespace re;

CTFInventoryManager *CTFInventoryManager::GTFInventoryManager()
{
    typedef CTFInventoryManager *(*GTFInventoryManager_t)();
    uintptr_t address = gSignatures.GetClientSignature(
        XORSTR("55 A1 ? ? ? ? 89 E5 5D C3 8D B6 00 00 00 00 55 89 E5 56 53 83 EC ? 8B ")
        XORSTR("5D ? C7 44 24"));
    GTFInventoryManager_t GTFInventoryManager_fn =
        GTFInventoryManager_t(address);
    return GTFInventoryManager_fn();
}
bool CTFInventoryManager::EquipItemInLoadout(int slot, int classid,
                                             unsigned long long uniqueid)
{
    typedef bool (*fn_t)(void *, int, int, unsigned long long);
    return vfunc<fn_t>(this,
                       offsets::PlatformOffset(19, offsets::undefined, 19),
                       0)(this, slot, classid, uniqueid);
}
unsigned long long int CEconItem::uniqueid()
{
    return *((unsigned long long int *) this + 36);
}
CTFPlayerInventory *CTFPlayerInventory::GTFPlayerInventory()
{
    typedef CTFPlayerInventory *(*GTFPlayerInventory_t)();
    uintptr_t address = gSignatures.GetClientSignature(
        XORSTR("55 B8 ? ? ? ? 89 E5 5D C3 8D B6 00 00 00 00 55 B8 ? ? ? ? 89 E5 5D C3 ")
        XORSTR("8D B6 00 00 00 00 55 89 E5 57 56 53 83 EC ? 8B 45 ? 8B 5D"));
    GTFPlayerInventory_t GTFPlayerInventory_fn = GTFPlayerInventory_t(address);
    return GTFPlayerInventory_fn();
}
CEconItem *CTFPlayerInventory::GetFirstItemOfItemDef(int id)
{
    typedef CEconItem *(*GetFirstItemOfItemDef_t)(CTFPlayerInventory *, int);
    uintptr_t address = gSignatures.GetClientSignature(
        XORSTR("55 89 E5 57 56 53 83 EC ? 8B 4D ? 0F B7 45"));
    GetFirstItemOfItemDef_t GetFirstItemOfItemDef_fn =
        GetFirstItemOfItemDef_t(address);
    return GetFirstItemOfItemDef_fn(this, id);
}
