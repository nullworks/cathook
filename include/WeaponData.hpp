#pragma once

#include "common.hpp"

struct WeaponData_t
{
    int m_nDamage;
    int m_nBulletsPerShot;
    float m_flRange;
    float m_flSpread;
    float m_flPunchAngle;
    float m_flTimeFireDelay;   // Time to delay between firing
    float m_flTimeIdle;        // Time to idle after firing
    float m_flTimeIdleEmpty;   // Time to idle after firing last bullet in clip
    float m_flTimeReloadStart; // Time to start into a reload (ie. shotgun)
    float m_flTimeReload;      // Time to reload
    bool m_bDrawCrosshair;     // Should the weapon draw a crosshair
    int m_iProjectile;         // The type of projectile this mode fires
    int m_iAmmoPerShot;        // How much ammo each shot consumes
    float m_flProjectileSpeed; // Start speed for projectiles (nail, etc.); NOTE: union with something non-projectile
    float m_flSmackDelay;      // how long after swing should damage happen for melee weapons
    bool m_bUseRapidFireCrits;
};

class weapon_info
{
public:
    float crit_bucket{};
    unsigned int weapon_seed{};
    unsigned unknown1{};
    unsigned unknown2{};
    bool unknown3{};
    float unknown4{};
    int crit_attempts{};
    int crit_count{};
    float observed_crit_chance{};
    bool unknown7{};
    int weapon_mode{};
    int weapon_data{};
    weapon_info()
    {
    }
    void Load(IClientEntity *weapon)
    {
        crit_bucket          = *(float *) ((uintptr_t) weapon + 0xa38);
        weapon_seed          = *(unsigned int *) ((uintptr_t) weapon + 0xb3c);
        unknown1             = *(unsigned int *) ((uintptr_t) weapon + 0xb30);
        unknown2             = *(unsigned int *) ((uintptr_t) weapon + 0xb34);
        unknown3             = *(bool *) ((uintptr_t) weapon + 0xb17);
        unknown4             = *(float *) ((uintptr_t) weapon + 0xb40);
        crit_attempts        = *(int *) ((uintptr_t) weapon + 0xa3c);
        crit_count           = *(int *) ((uintptr_t) weapon + 0xa40);
        observed_crit_chance = *(float *) ((uintptr_t) weapon + 0xbfc);
        unknown7             = *(bool *) ((uintptr_t) weapon + 0xb18);
        // No need to restore
        weapon_mode = *(int *) ((uintptr_t) weapon + 0xb04);
        weapon_data = *(int *) ((uintptr_t) weapon + 0xb10);
    }
    weapon_info(IClientEntity *weapon)
    {
        Load(weapon);
    }
    void restore_data(IClientEntity *weapon)
    {
        *(float *) ((uintptr_t) weapon + 0xa38)        = crit_bucket;
        *(unsigned int *) ((uintptr_t) weapon + 0xb3c) = weapon_seed;
        *(unsigned int *) ((uintptr_t) weapon + 0xb30) = unknown1;
        *(unsigned int *) ((uintptr_t) weapon + 0xb34) = unknown2;
        *(bool *) ((uintptr_t) weapon + 0xb17)         = unknown3;
        *(float *) ((uintptr_t) weapon + 0xb40)        = unknown4;
        *(int *) ((uintptr_t) weapon + 0xa3c)          = crit_attempts;
        *(int *) ((uintptr_t) weapon + 0xa40)          = crit_count;
        *(float *) ((uintptr_t) weapon + 0xbfc)        = observed_crit_chance;
        *(bool *) ((uintptr_t) weapon + 0xb18)         = unknown7;
    }
};

inline WeaponData_t *GetWeaponData(IClientEntity *weapon)
{
    weapon_info info(weapon);
    int WeaponData = info.weapon_data;
    int WeaponMode = info.weapon_mode;
    return (WeaponData_t *) (WeaponData + sizeof(WeaponData_t) * WeaponMode + 1784);
}
