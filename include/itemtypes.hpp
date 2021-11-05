/*
 * itemtypes.h
 *
 *  Created on: Feb 10, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include <unordered_map>
#include <string>
#include <vector>

enum k_EItemType
{
    ITEM_NONE = 0,

    ITEM_HEALTH_SMALL,
    ITEM_HEALTH_MEDIUM,
    ITEM_HEALTH_LARGE,
    EDIBLE_MEDIUM,
    EDIBLE_SMALL,

    ITEM_AMMO_SMALL,
    ITEM_AMMO_MEDIUM,
    ITEM_AMMO_LARGE,
    ITEM_CRUMPKIN,

    ITEM_POWERUP_STRENGTH,
    ITEM_POWERUP_RESISTANCE,
    ITEM_POWERUP_VAMPIRE,
    ITEM_POWERUP_REFLECT,
    ITEM_POWERUP_HASTE,
    ITEM_POWERUP_REGENERATION,
    ITEM_POWERUP_PRECISION,
    ITEM_POWERUP_AGILITY,
    ITEM_POWERUP_KNOCKOUT,
    ITEM_POWERUP_KING,
    ITEM_POWERUP_PLAGUE,
    ITEM_POWERUP_SUPERNOVA,
    ITEM_POWERUP_CRITS,

    ITEM_POWERUP_FIRST = ITEM_POWERUP_STRENGTH,
    ITEM_POWERUP_LAST  = ITEM_POWERUP_CRITS,

    HALLOWEEN_GHOST,

    BOMB_BALLOONBOMB,
    BOMB_WOODENBARREL,
    BOMB_WALKEREXPLODE,

    FLAG_ATOMBOMB,
    FLAG_SKULLPICKUP,
    FLAG_GIBBUCKET,
    FLAG_BOTTLEPICKUP,
    FLAG_GIFT,
    FLAG_AUSSIECONTAINER,
    FLAG_TICKETCASE,

    CART_BOMBCART,
    CART_BOMBCART_RED,

    ITEM_SPELL,
    ITEM_SPELL_RARE,

    ITEM_COUNT
};

class CachedEntity;
typedef bool (*ItemCheckerFn)(CachedEntity *);
typedef k_EItemType (*ItemSpecialMapperFn)(CachedEntity *);

class ItemModelMapper
{
public:
    void RegisterItem(std::string modelpath, k_EItemType type);
    k_EItemType GetItemType(CachedEntity *entity);

    std::unordered_map<std::string, k_EItemType> models;
    std::unordered_map<uintptr_t, k_EItemType> map;
};

class ItemManager
{
public:
    ItemManager();
    void RegisterModelMapping(std::string path, k_EItemType type);
    void RegisterSpecialMapping(ItemCheckerFn fn, k_EItemType type);
    k_EItemType GetItemType(CachedEntity *ent);

    std::unordered_map<ItemCheckerFn, k_EItemType> special_map;
    std::vector<ItemSpecialMapperFn> specials;
    ItemModelMapper mapper_special;
    ItemModelMapper mapper;
};

extern ItemManager g_ItemManager;
