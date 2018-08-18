target_sources(cathook PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/C_BaseCombatWeapon.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/C_BaseEntity.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/C_BasePlayer.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/C_MannVsMachineStats.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/C_TEFireBullets.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/CTFGCClientSystem.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/CTFInventoryManager.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/CTFPartyClient.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/CTFParty.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/CTFPlayerShared.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/C_TFWeaponBaseGun.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/C_TFWeaponBase.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/C_TFWeaponBaseMelee.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/ITFGroupMatchCriteria.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/reclasses.hpp")

target_include_directories(cathook PRIVATE "${CMAKE_CURRENT_LIST_DIR}")