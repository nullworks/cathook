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
    RegisterModelMapping(XORSTR("models/items/medkit_small.mdl"), ITEM_HEALTH_SMALL);
    RegisterModelMapping(XORSTR("models/items/medkit_medium.mdl"), ITEM_HEALTH_MEDIUM);
    RegisterModelMapping(XORSTR("models/items/medkit_large.mdl"), ITEM_HEALTH_LARGE);
    // Halloween
    RegisterModelMapping(XORSTR("models/props_halloween/halloween_medkit_small.mdl"),
                         ITEM_HEALTH_SMALL);
    RegisterModelMapping(XORSTR("models/props_halloween/halloween_medkit_medium.mdl"),
                         ITEM_HEALTH_MEDIUM);
    RegisterModelMapping(XORSTR("models/props_halloween/halloween_medkit_large.mdl"),
                         ITEM_HEALTH_LARGE);
    // Holiday
    RegisterModelMapping(XORSTR("models/items/medkit_small_bday.mdl"),
                         ITEM_HEALTH_SMALL);
    RegisterModelMapping(XORSTR("models/items/medkit_medium_bday.mdl"),
                         ITEM_HEALTH_MEDIUM);
    RegisterModelMapping(XORSTR("models/items/medkit_large_bday.mdl"),
                         ITEM_HEALTH_LARGE);
    // Medieval
    RegisterModelMapping(XORSTR("models/props_medieval/medieval_meat.mdl"),
                         ITEM_HEALTH_MEDIUM);

    // == AMMOPACKS
    // Normal
    RegisterModelMapping(XORSTR("models/items/ammopack_small.mdl"), ITEM_AMMO_SMALL);
    RegisterModelMapping(XORSTR("models/items/ammopack_medium.mdl"), ITEM_AMMO_MEDIUM);
    RegisterModelMapping(XORSTR("models/items/ammopack_large.mdl"), ITEM_AMMO_LARGE);
    // Holiday
    RegisterModelMapping(XORSTR("models/items/ammopack_small_bday.mdl"),
                         ITEM_AMMO_SMALL);
    RegisterModelMapping(XORSTR("models/items/ammopack_medium_bday.mdl"),
                         ITEM_AMMO_MEDIUM);
    RegisterModelMapping(XORSTR("models/items/ammopack_large_bday.mdl"),
                         ITEM_AMMO_LARGE);

    // == POWERUPS
    RegisterModelMapping(XORSTR("models/pickups/pickup_powerup_haste.mdl"),
                         ITEM_POWERUP_HASTE);
    RegisterModelMapping(XORSTR("models/pickups/pickup_powerup_vampire.mdl"),
                         ITEM_POWERUP_VAMPIRE);
    RegisterModelMapping(XORSTR("models/pickups/pickup_powerup_precision.mdl"),
                         ITEM_POWERUP_PRECISION);
    // RegisterModelMapping(XORSTR("models/pickups/pickup_powerup_strength_arm.mdl"),
    // ITEM_POWERUP_AGILITY);
    RegisterModelMapping(XORSTR("models/pickups/pickup_powerup_regen.mdl"),
                         ITEM_POWERUP_REGENERATION);
    RegisterModelMapping(XORSTR("models/pickups/pickup_powerup_supernova.mdl"),
                         ITEM_POWERUP_SUPERNOVA);
    RegisterModelMapping(XORSTR("models/pickups/pickup_powerup_strength.mdl"),
                         ITEM_POWERUP_STRENGTH);
    // RegisterModelMapping(XORSTR("models/pickups/pickup_powerup_resistance.mdl"),
    // ITEM_POWERUP_RESISTANCE);
    RegisterModelMapping(XORSTR("models/pickups/pickup_powerup_knockout.mdl"),
                         ITEM_POWERUP_KNOCKOUT);
    // RegisterModelMapping(XORSTR("models/pickups/pickup_powerup_uber.mdl"),
    // ITEM_POWERUP_AGILITY);
    RegisterModelMapping(XORSTR("models/pickups/pickup_powerup_defense.mdl"),
                         ITEM_POWERUP_RESISTANCE);
    RegisterModelMapping(XORSTR("models/pickups/pickup_powerup_crit.mdl"),
                         ITEM_POWERUP_CRITS);
    RegisterModelMapping(XORSTR("models/pickups/pickup_powerup_agility.mdl"),
                         ITEM_POWERUP_AGILITY);
    RegisterModelMapping(XORSTR("models/pickups/pickup_powerup_king.mdl"),
                         ITEM_POWERUP_KING);
    // RegisterModelMapping(XORSTR("models/pickups/pickup_powerup_warlock.mdl"),
    // ITEM_POWERUP_REFLECT);
    RegisterModelMapping(XORSTR("models/pickups/pickup_powerup_plague.mdl"),
                         ITEM_POWERUP_PLAGUE);
    RegisterModelMapping(XORSTR("models/pickups/pickup_powerup_reflect.mdl"),
                         ITEM_POWERUP_REFLECT);
    // RegisterModelMapping(XORSTR("models/pickups/pickup_powerup_thorns.mdl"),
    // ITEM_POWERUP_AGILITY);

    RegisterModelMapping(XORSTR("models/items/battery.mdl"), ITEM_HL_BATTERY);
    RegisterModelMapping(XORSTR("models/items/healthkit.mdl"), ITEM_HEALTH_MEDIUM);
    // RegisterModelMapping(XORSTR("models/pickups/pickup_powerup_reflect.mdl"),
    // ITEM_POWERUP_REFLECT);

    // Spellbooks
    RegisterModelMapping(XORSTR("models/props_halloween/hwn_spellbook_upright.mdl"),
                         ITEM_SPELL);
    RegisterModelMapping(
        XORSTR("models/props_halloween/hwn_spellbook_upright_major.mdl"),
        ITEM_SPELL_RARE);

    RegisterSpecialMapping(
        [](CachedEntity *ent) -> bool {
            return ent->m_iClassID() == CL_CLASS(CTFAmmoPack);
        },
        ITEM_AMMO_MEDIUM);

    RegisterModelMapping(XORSTR("models/items/medkit_overheal.mdl"), ITEM_TF2C_PILL);
    // TF2C spawners

    specials.push_back([](CachedEntity *entity) -> k_EItemType {
        static ItemModelMapper tf2c_weapon_mapper = []() -> ItemModelMapper {
            ItemModelMapper result;
            result.RegisterItem(XORSTR("models/weapons/w_models/w_bottle.mdl"),
                                ITEM_TF2C_W_BOTTLE);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_grenade_napalm.mdl"),
                                ITEM_TF2C_W_GRENADE_NAPALM);
            result.RegisterItem(
                XORSTR("models/weapons/w_models/w_supershotgun_mercenary.mdl"),
                ITEM_TF2C_W_SUPERSHOTGUN_MERCENARY);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_rocketbeta.mdl"),
                                ITEM_TF2C_W_ROCKETBETA);
            result.RegisterItem(
                XORSTR("models/weapons/w_models/w_grenade_grenadelauncher.mdl"),
                ITEM_TF2C_W_GRENADE_GRENADELAUNCHER);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_grenadelauncher.mdl"),
                                ITEM_TF2C_W_GRENADELAUNCHER);
            result.RegisterItem(
                XORSTR("models/weapons/w_models/w_grenade_pipebomb.mdl"),
                ITEM_TF2C_W_GRENADE_PIPEBOMB);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_cigarette_case.mdl"),
                                ITEM_TF2C_W_CIGARETTE_CASE);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_brandingiron.mdl"),
                                ITEM_TF2C_W_BRANDINGIRON);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_banhammer.mdl"),
                                ITEM_TF2C_W_BANHAMMER);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_sniperrifle.mdl"),
                                ITEM_TF2C_W_SNIPERRIFLE);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_grenade_heal.mdl"),
                                ITEM_TF2C_W_GRENADE_HEAL);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_hammerfists.mdl"),
                                ITEM_TF2C_W_HAMMERFISTS);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_grenade_nail.mdl"),
                                ITEM_TF2C_W_GRENADE_NAIL);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_dart.mdl"),
                                ITEM_TF2C_W_DART);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_rpg.mdl"),
                                ITEM_TF2C_W_RPG);
            result.RegisterItem(
                XORSTR("models/weapons/w_models/w_umbrella_civilian.mdl"),
                ITEM_TF2C_W_UMBRELLA_CIVILIAN);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_grenade_frag.mdl"),
                                ITEM_TF2C_W_GRENADE_FRAG);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_grenade_mirv.mdl"),
                                ITEM_TF2C_W_GRENADE_MIRV);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_medigun.mdl"),
                                ITEM_TF2C_W_MEDIGUN);
            result.RegisterItem(
                XORSTR("models/weapons/w_models/w_grenade_mirv_demo.mdl"),
                ITEM_TF2C_W_GRENADE_MIRV_DEMO);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_pickaxe.mdl"),
                                ITEM_TF2C_W_PICKAXE);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_syringegun.mdl"),
                                ITEM_TF2C_W_SYRINGEGUN);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_scattergun.mdl"),
                                ITEM_TF2C_W_SCATTERGUN);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_stengun.mdl"),
                                ITEM_TF2C_W_STENGUN);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_snubnose.mdl"),
                                ITEM_TF2C_W_SNUBNOSE);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_minigun.mdl"),
                                ITEM_TF2C_W_MINIGUN);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_wrench.mdl"),
                                ITEM_TF2C_W_WRENCH);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_bat.mdl"),
                                ITEM_TF2C_W_BAT);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_sapper.mdl"),
                                ITEM_TF2C_W_SAPPER);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_rocket.mdl"),
                                ITEM_TF2C_W_ROCKET);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_hammer.mdl"),
                                ITEM_TF2C_W_HAMMER);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_fishwhacker.mdl"),
                                ITEM_TF2C_W_FISHWHACKER);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_kritzkrieg.mdl"),
                                ITEM_TF2C_W_KRITZKRIEG);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_flaregun.mdl"),
                                ITEM_TF2C_W_FLAREGUN);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_shovel.mdl"),
                                ITEM_TF2C_W_SHOVEL);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_pistol.mdl"),
                                ITEM_TF2C_W_PISTOL);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_smg.mdl"),
                                ITEM_TF2C_W_SMG);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_leadpipe.mdl"),
                                ITEM_TF2C_W_LEADPIPE);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_flaregun_shell.mdl"),
                                ITEM_TF2C_W_FLAREGUN_SHELL);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_grenade_emp.mdl"),
                                ITEM_TF2C_W_GRENADE_EMP);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_cyclops.mdl"),
                                ITEM_TF2C_W_CYCLOPS);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_machete.mdl"),
                                ITEM_TF2C_W_MACHETE);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_shotgun.mdl"),
                                ITEM_TF2C_W_SHOTGUN);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_revolver.mdl"),
                                ITEM_TF2C_W_REVOLVER);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_grenade_gas.mdl"),
                                ITEM_TF2C_W_GRENADE_GAS);
            result.RegisterItem(
                XORSTR("models/weapons/w_models/w_grenade_beartrap.mdl"),
                ITEM_TF2C_W_GRENADE_BEARTRAP);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_fireaxe.mdl"),
                                ITEM_TF2C_W_FIREAXE);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_grenade_conc.mdl"),
                                ITEM_TF2C_W_GRENADE_CONC);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_nailgun.mdl"),
                                ITEM_TF2C_W_NAILGUN);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_bonesaw.mdl"),
                                ITEM_TF2C_W_BONESAW);
            result.RegisterItem(
                XORSTR("models/weapons/w_models/w_stickybomb_launcher.mdl"),
                ITEM_TF2C_W_STICKYBOMB_LAUNCHER);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_grenade_bomblet.mdl"),
                                ITEM_TF2C_W_GRENADE_BOMBLET);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_tranq.mdl"),
                                ITEM_TF2C_W_TRANQ);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_dynamite.mdl"),
                                ITEM_TF2C_W_DYNAMITE);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_club.mdl"),
                                ITEM_TF2C_W_CLUB);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_tommygun.mdl"),
                                ITEM_TF2C_W_TOMMYGUN);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_coffepot.mdl"),
                                ITEM_TF2C_W_COFFEPOT);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_stickybomb.mdl"),
                                ITEM_TF2C_W_STICKYBOMB);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_rocketlauncher.mdl"),
                                ITEM_TF2C_W_ROCKETLAUNCHER);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_flamethrower.mdl"),
                                ITEM_TF2C_W_FLAMETHROWER);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_crowbar.mdl"),
                                ITEM_TF2C_W_CROWBAR);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_builder.mdl"),
                                ITEM_TF2C_W_BUILDER);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_heavy_artillery.mdl"),
                                ITEM_TF2C_W_HEAVY_ARTILLERY);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_knife.mdl"),
                                ITEM_TF2C_W_KNIFE);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_pda_engineer.mdl"),
                                ITEM_TF2C_W_PDA_ENGINEER);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_bottle_broken.mdl"),
                                ITEM_TF2C_W_BOTTLE_BROKEN);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_toolbox.mdl"),
                                ITEM_TF2C_W_TOOLBOX);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_syringe_proj.mdl"),
                                ITEM_TF2C_W_SYRINGE_PROJ);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_nail.mdl"),
                                ITEM_TF2C_W_NAIL);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_syringe.mdl"),
                                ITEM_TF2C_W_SYRINGE);
            result.RegisterItem(XORSTR("models/weapons/w_models/w_overhealer.mdl"),
                                ITEM_TF2C_W_OVERHEALER);
            return result;
        }();

        if (entity->m_iClassID() == CL_CLASS(CWeaponSpawner))
        {
            return tf2c_weapon_mapper.GetItemType(entity);
        }
        return ITEM_NONE;
    });
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
    for (const auto &it : map)
    {
        if (it.first == model)
            return it.second;
    }
    std::string path(g_IModelInfo->GetModelName((const model_t *) model));
    bool set = false;
    for (const auto &it : models)
    {
        // logging::Info(XORSTR("comparing [%s] to [%s]"), path.c_str(),
        // it.first.c_str());
        if (it.first == path)
        {
            // logging::Info(XORSTR("Found %s!"), path.c_str());
            map.emplace(model, it.second);
            set = true;
            break;
        }
    }
    if (!set)
        map.emplace(model, k_EItemType::ITEM_NONE);
    return k_EItemType::ITEM_NONE;
}

ItemManager g_ItemManager;
