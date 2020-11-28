set(files "${CMAKE_CURRENT_LIST_DIR}/AutoJoin.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/CatBot.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Spam.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoItem.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Aimbot.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoViewmodel.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/MiscAimbot.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Announcer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AntiAim.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AntiAntiAim.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AntiBackstab.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AntiCheat.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AntiDisguise.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoBackstab.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoDeadringer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoDetonator.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoHeal.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoParty.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoReflect.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoSticky.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoTaunt.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Bunnyhop.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Backtrack.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/BPExpander.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ChatCommands.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/CritSay.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/DataCenter.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/FollowBot.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/KillSay.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/DominateSay.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Killstreak.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/LightESP.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Misc.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/MiscPlayerInfo.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/NavBot.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Noisemaker.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/PureBypass.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Trigger.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/UberSpam.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Walkbot.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Warp.cpp")
target_sources(cathook PRIVATE ${files})
list(REMOVE_ITEM ignore_files ${files})
set(ignore_files ${ignore_files} CACHE INTERNAL "")

add_subdirectory(ac)
if(EnableVisuals)
    set(files "${CMAKE_CURRENT_LIST_DIR}/BulletTracers.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/DominateMark.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/ESP.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/ExplosionCircles.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/KillfeedColor.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/Tracers.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/Radar.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/SkinChanger.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/SpyAlert.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/Thirdperson.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/MCHealthbar.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/Ragdolls.cpp")
    target_sources(cathook PRIVATE ${files})
    list(REMOVE_ITEM ignore_files ${files})
    set(ignore_files ${ignore_files} CACHE INTERNAL "")
endif()
