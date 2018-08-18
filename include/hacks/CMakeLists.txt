target_sources(cathook PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/AutoJoin.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/CatBot.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/SeedPrediction.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Spam.hpp")
if(NOT LagbotMode)
target_sources(cathook PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/Achievement.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Aimbot.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Announcer.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/AntiAim.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/AntiAntiAim.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/AntiBackstab.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/AntiCheat.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/AntiDisguise.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoBackstab.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoDeadringer.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoDetonator.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoHeal.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoJoin.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoReflect.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoSticky.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/AutoTaunt.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Bunnyhop.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Backtrack.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/CatBot.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/FollowBot.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/hacklist.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/KillSay.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Killstreak.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/LightESP.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Misc.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/NavBot.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Noisemaker.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Spam.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Trigger.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/UberSpam.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Walkbot.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Radar.hpp")
        add_subdirectory(ac)
endif()
if(EnableVisuals)
target_sources(cathook PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/ESP.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Radar.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/SkinChanger.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Thirdperson.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/SpyAlert.hpp")
endif()

target_include_directories(cathook PRIVATE "${CMAKE_CURRENT_LIST_DIR}")
