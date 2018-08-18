target_sources(cathook PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/angles.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/averager.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/backpacktf.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/base64.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/chatlog.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/chatstack.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/common.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/conditions.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/crits.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/entitycache.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/entityhitboxcache.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/enums.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/gameinfo.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/globals.h"
        "${CMAKE_CURRENT_LIST_DIR}/hack.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/headshake.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/helpers.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/hitrate.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/hooks.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/hoovy.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/https_request.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/init.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/ipc.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/itemtypes.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/json.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/localplayer.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/playerlist.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/playerresource.h"
        "${CMAKE_CURRENT_LIST_DIR}/prediction.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/projlogging.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/sconvars.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/targethelper.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/textfile.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/textmode.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/tfmm.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/timer.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/trace.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/velocity.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/votelogger.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/MiscTemporary.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/navparser.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Options.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/PlayerTools.hpp")

target_include_directories(cathook PRIVATE "${CMAKE_CURRENT_LIST_DIR}")

add_subdirectory(core)
add_subdirectory(classinfo)
add_subdirectory(copypasted)
add_subdirectory(hacks)
add_subdirectory(hooks)
add_subdirectory(reclasses)
add_subdirectory(sdk)
add_subdirectory(online)
add_subdirectory(settings)

if(EnableVisuals)
    add_subdirectory(visual)
endif()

