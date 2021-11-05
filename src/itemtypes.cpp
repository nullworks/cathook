/*
 * itemtypes.cpp
 *
 *  Created on: Feb 10, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"

ItemManager::ItemManager() : mapper()
{
    // == MEDKITS
    // Normal
    RegisterModelMapping("models/items/medkit_small.mdl", ITEM_HEALTH_SMALL);
    RegisterModelMapping("models/items/medkit_medium.mdl", ITEM_HEALTH_MEDIUM);
    RegisterModelMapping("models/items/medkit_large.mdl", ITEM_HEALTH_LARGE);
    // Halloween
    RegisterModelMapping("models/props_halloween/halloween_medkit_small.mdl", ITEM_HEALTH_SMALL);
    RegisterModelMapping("models/props_halloween/halloween_medkit_medium.mdl", ITEM_HEALTH_MEDIUM);
    RegisterModelMapping("models/props_halloween/halloween_medkit_large.mdl", ITEM_HEALTH_LARGE);
    // Holiday
    RegisterModelMapping("models/items/medkit_small_bday.mdl", ITEM_HEALTH_SMALL);
    RegisterModelMapping("models/items/medkit_medium_bday.mdl", ITEM_HEALTH_MEDIUM);
    RegisterModelMapping("models/items/medkit_large_bday.mdl", ITEM_HEALTH_LARGE);
    // Medieval
    RegisterModelMapping("models/props_medieval/medieval_meat.mdl", ITEM_HEALTH_MEDIUM);
    // Edibles / Lunchboxes
    RegisterModelMapping("models/items/plate.mdl", EDIBLE_MEDIUM);
    RegisterModelMapping("models/items/plate_sandwich_xmas.mdl", EDIBLE_MEDIUM);
    RegisterModelMapping("models/items/plate_robo_sandwich.mdl", EDIBLE_MEDIUM);
    RegisterModelMapping("models/workshop/weapons/c_models/c_fishcake/plate_fishcake.mdl", EDIBLE_SMALL);
    RegisterModelMapping("models/workshop/weapons/c_models/c_buffalo_steak/plate_buffalo_steak.mdl", EDIBLE_SMALL);
    RegisterModelMapping("models/workshop/weapons/c_models/c_chocolate/plate_chocolate.mdl", EDIBLE_SMALL);
    RegisterModelMapping("models/items/banana/plate_banana.mdl", EDIBLE_SMALL);

    // == AMMOPACKS
    // Normal
    RegisterModelMapping("models/items/ammopack_small.mdl", ITEM_AMMO_SMALL);
    RegisterModelMapping("models/items/ammopack_medium.mdl", ITEM_AMMO_MEDIUM);
    RegisterModelMapping("models/items/ammopack_large.mdl", ITEM_AMMO_LARGE);
    // Holiday
    RegisterModelMapping("models/items/ammopack_small_bday.mdl", ITEM_AMMO_SMALL);
    RegisterModelMapping("models/items/ammopack_medium_bday.mdl", ITEM_AMMO_MEDIUM);
    RegisterModelMapping("models/items/ammopack_large_bday.mdl", ITEM_AMMO_LARGE);
    // Crumpkin
    RegisterModelMapping("models/props_halloween/pumpkin_loot.mdl", ITEM_CRUMPKIN);

    // == POWERUPS
    RegisterModelMapping("models/pickups/pickup_powerup_haste.mdl", ITEM_POWERUP_HASTE);
    RegisterModelMapping("models/pickups/pickup_powerup_vampire.mdl", ITEM_POWERUP_VAMPIRE);
    RegisterModelMapping("models/pickups/pickup_powerup_precision.mdl", ITEM_POWERUP_PRECISION);
    // RegisterModelMapping("models/pickups/pickup_powerup_strength_arm.mdl",
    // ITEM_POWERUP_AGILITY);
    RegisterModelMapping("models/pickups/pickup_powerup_regen.mdl", ITEM_POWERUP_REGENERATION);
    RegisterModelMapping("models/pickups/pickup_powerup_supernova.mdl", ITEM_POWERUP_SUPERNOVA);
    RegisterModelMapping("models/pickups/pickup_powerup_strength.mdl", ITEM_POWERUP_STRENGTH);
    // RegisterModelMapping("models/pickups/pickup_powerup_resistance.mdl",
    // ITEM_POWERUP_RESISTANCE);
    RegisterModelMapping("models/pickups/pickup_powerup_knockout.mdl", ITEM_POWERUP_KNOCKOUT);
    // RegisterModelMapping("models/pickups/pickup_powerup_uber.mdl",
    // ITEM_POWERUP_AGILITY);
    RegisterModelMapping("models/pickups/pickup_powerup_defense.mdl", ITEM_POWERUP_RESISTANCE);
    RegisterModelMapping("models/pickups/pickup_powerup_crit.mdl", ITEM_POWERUP_CRITS);
    RegisterModelMapping("models/pickups/pickup_powerup_agility.mdl", ITEM_POWERUP_AGILITY);
    RegisterModelMapping("models/pickups/pickup_powerup_king.mdl", ITEM_POWERUP_KING);
    // RegisterModelMapping("models/pickups/pickup_powerup_warlock.mdl",
    // ITEM_POWERUP_REFLECT);
    RegisterModelMapping("models/pickups/pickup_powerup_plague.mdl", ITEM_POWERUP_PLAGUE);
    RegisterModelMapping("models/pickups/pickup_powerup_reflect.mdl", ITEM_POWERUP_REFLECT);
    // RegisterModelMapping("models/pickups/pickup_powerup_thorns.mdl",
    // ITEM_POWERUP_AGILITY);

    // Spellbooks
    RegisterModelMapping("models/props_halloween/hwn_spellbook_upright.mdl", ITEM_SPELL);
    RegisterModelMapping("models/items/crystal_ball_pickup.mdl", ITEM_SPELL);
    RegisterModelMapping("models/props_monster_mash/flask_vial_green.mdl", ITEM_SPELL);
    RegisterModelMapping("models/props_halloween/hwn_spellbook_upright_major.mdl", ITEM_SPELL_RARE);
    RegisterModelMapping("models/items/crystal_ball_pickup_major.mdl", ITEM_SPELL_RARE);
    RegisterModelMapping("models/props_monster_mash/flask_vial_purple.mdl", ITEM_SPELL_RARE);

    // Thanks valve
    RegisterSpecialMapping([](CachedEntity *ent) -> bool { return g_ItemManager.mapper.GetItemType(ent) == ITEM_SPELL && ent->m_iClassID() == CL_CLASS(CDynamicProp); }, ITEM_SPELL_RARE);

    // == GHOSTS
    RegisterModelMapping("models/props_halloween/ghost.mdl", HALLOWEEN_GHOST);
    RegisterModelMapping("models/props_halloween/ghost_no_hat_red.mdl", HALLOWEEN_GHOST);
    RegisterModelMapping("models/props_halloween/ghost_no_hat.mdl", HALLOWEEN_GHOST);

    // == BOMBS
    RegisterModelMapping("models/props_laughter/balloonbomb.mdl", BOMB_BALLOONBOMB);
    RegisterModelMapping("models/props_coast/wooden_barrel.mdl", BOMB_WOODENBARREL);
    RegisterModelMapping("models/props_invasion/props_alien/walker_explode.mdl", BOMB_WALKEREXPLODE);

    // == FLAGS
    RegisterModelMapping("models/props_td/atom_bomb.mdl", FLAG_ATOMBOMB);
    RegisterModelMapping("models/props_pirate/pd_skull_pickup.mdl", FLAG_SKULLPICKUP);
    RegisterModelMapping("models/effects/playersoul.mdl", FLAG_SKULLPICKUP);
    RegisterModelMapping("models/props_monster_mash/gib_bucket.mdl", FLAG_GIBBUCKET);
    RegisterModelMapping("models/props_watergate/bottle_pickup.mdl", FLAG_BOTTLEPICKUP);
    RegisterModelMapping("models/props_halloween/halloween_gift.mdl", FLAG_GIFT);
    RegisterModelMapping("models/props_doomsday/australium_container.mdl", FLAG_AUSSIECONTAINER);
    RegisterModelMapping("models/flag/ticket_case.mdl", FLAG_TICKETCASE);

    // == BOMB CARTS
    RegisterModelMapping("models/props_trainyard/bomb_cart.mdl", CART_BOMBCART);
    RegisterModelMapping("models/custom/dirty_bomb_cart.mdl", CART_BOMBCART);
    RegisterModelMapping("models/lilchewchew/lilchewchew_v3.mdl", CART_BOMBCART);
    RegisterModelMapping("models/props_trainyard/bomb_redmond.mdl", CART_BOMBCART);
    RegisterModelMapping("models/props_snowycoast/gasoline_bomb_cart.mdl", CART_BOMBCART);
    RegisterModelMapping("models/props_xmas/rudy.mdl", CART_BOMBCART);
    RegisterModelMapping("models/props_trainyard/bomb_blutarch.mdl", CART_BOMBCART_RED);
    RegisterModelMapping("models/props_trainyard/bomb_cart_red.mdl", CART_BOMBCART_RED);

    RegisterSpecialMapping([](CachedEntity *ent) -> bool { return ent->m_iClassID() == CL_CLASS(CTFAmmoPack) && g_ItemManager.mapper.GetItemType(ent) != ITEM_CRUMPKIN; }, ITEM_AMMO_MEDIUM);
}

void ItemManager::RegisterModelMapping(std::string path, k_EItemType type)
{
    mapper.RegisterItem(path, type);
}

void ItemManager::RegisterSpecialMapping(ItemCheckerFn fn, k_EItemType type)
{
    special_map.emplace(fn, type);
}

k_EItemType ItemManager::GetItemType(CachedEntity *ent)
{
    for (const auto &it : specials)
    {
        const auto type = it(ent);
        if (type != ITEM_NONE)
            return type;
    }
    for (const auto &it : special_map)
    {
        if (it.first(ent))
            return it.second;
    }
    return mapper.GetItemType(ent);
}

void ItemModelMapper::RegisterItem(std::string modelpath, k_EItemType type)
{
    models.emplace(modelpath, type);
}

k_EItemType ItemModelMapper::GetItemType(CachedEntity *entity)
{
    const uintptr_t model = (uintptr_t) RAW_ENT(entity)->GetModel();
    if (model)
    {
        for (const auto &it : map)
        {
            if (it.first == model)
                return it.second;
        }
        std::string path(g_IModelInfo->GetModelName((const model_t *) model));
        bool set = false;
        for (const auto &it : models)
        {
            // logging::Info("comparing [%s] to [%s]", path.c_str(),
            // it.first.c_str());
            if (it.first == path)
            {
                // logging::Info("Found %s!", path.c_str());
                map.emplace(model, it.second);
                set = true;
                break;
            }
        }
        if (!set)
            map.emplace(model, k_EItemType::ITEM_NONE);
    }
    return k_EItemType::ITEM_NONE;
}

ItemManager g_ItemManager;
static InitRoutine init_itemtypes(
    []()
    {
        EC::Register(
            EC::LevelInit, []() { g_ItemManager = ItemManager{}; }, "clear_itemtypes");
    });
