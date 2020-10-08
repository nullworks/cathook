/*
 * CTFInventoryManager.hpp
 *
 *  Created on: Apr 26, 2018
 *      Author: bencat07
 */
#pragma once

#include "common.hpp"
namespace re
{

class CEconItem
{
public:
    unsigned long long uniqueid();
};

class CEconItemView
{
public:
    int GetDefinitionIndex();
    unsigned long long UUID();
};

class CTFPlayerInventory
{
public:
    CTFPlayerInventory() = delete;

public:
    int GetItemCount();
    CEconItemView *GetItem(int idx);
    std::vector<unsigned long long> GetItemsOfItemDef(int id);
    CEconItemView *GetFirstItemOfItemDef(int id);
};

class CTFInventoryManager
{
public:
    CTFInventoryManager() = delete;
    static CTFInventoryManager *GTFInventoryManager();
    CTFPlayerInventory *GTFPlayerInventory();

public:
    bool EquipItemInLoadout(int, int, unsigned long long);
};
} // namespace re

extern bool Craft(std::vector<int> item_ids);
extern bool Rent(int item_id);
