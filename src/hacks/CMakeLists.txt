target_sources(cathook PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/AutoJoin.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/CatBot.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/SeedPrediction.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Spam.cpp")
if(NOT LagbotMode)
	target_sources(cathook PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/Achievement.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Aimbot.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Announcer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AntiAim.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AntiBackstab.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AntiCheat.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AntiDisguise.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoBackstab.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoDeadringer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoDetonator.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoEquip.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoHeal.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoReflect.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoSticky.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoTaunt.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Bunnyhop.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Backtrack.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/FollowBot.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/HealArrows.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/KillSay.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Killstreak.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/LagExploit.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/LightESP.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Misc.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Noisemaker.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Trigger.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/UberSpam.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Walkbot.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Radar.cpp")
        add_subdirectory(ac)
endif()
if(EnableVisuals)
    target_sources(cathook PRIVATE
            "${CMAKE_CURRENT_LIST_DIR}/ESP.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/Radar.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/SkinChanger.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/SpyAlert.cpp")
endif()
