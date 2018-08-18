/*
 * CTFInventoryManager.cpp
 *
 *  Created on: Apr 26, 2018
 *      Author: bencat07
 */
#pragma once

#include "common.hpp"
namespace re
{
class CTFInventoryManager
{
public:
    CTFInventoryManager() = delete;
    static CTFInventoryManager *GTFInventoryManager();

public:
    bool EquipItemInLoadout(int, int, unsigned long long);
};
class CEconItem
{
public:
    unsigned long long uniqueid();
};
class CTFPlayerInventory
{
public:
    CTFPlayerInventory() = delete;
    static CTFPlayerInventory *GTFPlayerInventory();

public:
    CEconItem *GetFirstItemOfItemDef(int id);
};
} // namespace re
