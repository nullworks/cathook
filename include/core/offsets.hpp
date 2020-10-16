/*
 * offsets.hpp
 *
 *  Created on: May 4, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include <stdint.h>
#include <exception>

enum class platform
{
    PLATFORM_LINUX,
    PLATFORM_WINDOWS,
    PLATFORM_OSX,
    PLATFORM_UNSUPPORTED
};

#ifdef LINUX
constexpr platform PLATFORM = platform::PLATFORM_LINUX;
#else
constexpr platform PLATFORM = platform::PLATFORM_UNSUPPORTED;
#endif

struct offsets
{
    static constexpr uint32_t undefined = std::numeric_limits<uint32_t>::max();

    static constexpr uint32_t PlatformOffset(uint32_t offset_linux, uint32_t offset_windows, uint32_t offset_osx)
    {
        uint32_t result = undefined;
        switch (PLATFORM)
        {
        case platform::PLATFORM_LINUX:
            result = offset_linux;
            break;
        case platform::PLATFORM_WINDOWS:
            result = offset_windows;
            break;
        case platform::PLATFORM_OSX:
            result = offset_osx;
            break;
        }
        // pCompileError.
        // static_assert(result != undefined, "No offset defined for this
        // platform!");
        return result;
    }
    static constexpr uint32_t GetUserCmd()
    {
        return PlatformOffset(8, undefined, undefined);
    }
    static constexpr uint32_t ShouldDraw()
    {
        return PlatformOffset(136, undefined, undefined);
    }
    static constexpr uint32_t DrawModelExecute()
    {
        return PlatformOffset(19, undefined, undefined);
    }
    static constexpr uint32_t GetClientName()
    {
        return PlatformOffset(44, undefined, undefined);
    }
    static constexpr uint32_t ProcessSetConVar()
    {
        return PlatformOffset(4, undefined, undefined);
    }
    static constexpr uint32_t ProcessMovement()
    {
        return PlatformOffset(1, undefined, undefined);
    }
    static constexpr uint32_t ProcessGetCvarValue()
    {
        return PlatformOffset(29, undefined, undefined);
    }
    static constexpr uint32_t GetFriendPersonaName()
    {
        return PlatformOffset(7, undefined, undefined);
    }
    static constexpr uint32_t CreateMoveInput()
    {
        return PlatformOffset(3, undefined, undefined);
    }
    static constexpr uint32_t CreateMove()
    {
        return PlatformOffset(22, undefined, undefined);
    }
    static constexpr uint32_t PaintTraverse()
    {
        return PlatformOffset(42, undefined, undefined);
    }
    static constexpr uint32_t OverrideView()
    {
        return PlatformOffset(17, undefined, undefined);
    }
    static constexpr uint32_t FrameStageNotify()
    {
        return PlatformOffset(35, undefined, undefined);
    }
    static constexpr uint32_t DispatchUserMessage()
    {
        return PlatformOffset(36, undefined, undefined);
    }
    static constexpr uint32_t CanPacket()
    {
        return PlatformOffset(57, undefined, undefined);
    }
    static constexpr uint32_t SendNetMsg()
    {
        return PlatformOffset(41, undefined, undefined);
    }
    static constexpr uint32_t Shutdown()
    {
        return PlatformOffset(37, undefined, undefined);
    }
    static constexpr uint32_t IN_KeyEvent()
    {
        return PlatformOffset(20, undefined, undefined);
    }
    static constexpr uint32_t HandleInputEvent()
    {
        return PlatformOffset(78, undefined, undefined);
    }
    static constexpr uint32_t LevelInit()
    {
        return PlatformOffset(23, undefined, undefined);
    }
    static constexpr uint32_t LevelShutdown()
    {
        return PlatformOffset(24, undefined, undefined);
    }
    static constexpr uint32_t BeginFrame()
    {
        return PlatformOffset(5, undefined, undefined);
    }
    static constexpr uint32_t FireGameEvent()
    {
        return PlatformOffset(2, undefined, undefined);
    }
    static constexpr uint32_t FireEvent()
    {
        return PlatformOffset(8, undefined, undefined);
    }
    static constexpr uint32_t FireEventClientSide()
    {
        return PlatformOffset(9, undefined, undefined);
    }
    static constexpr uint32_t AreRandomCritsEnabled()
    {
        return PlatformOffset(466, undefined, 466);
    }
    static constexpr uint32_t lastoutgoingcommand()
    {
        return PlatformOffset(19228, undefined, undefined);
    }
    static constexpr uint32_t m_nOutSequenceNr()
    {
        return PlatformOffset(8, undefined, undefined);
    }
    static constexpr uint32_t m_NetChannel()
    {
        return PlatformOffset(196, undefined, undefined);
    }
    static constexpr uint32_t RandomInt()
    {
        return PlatformOffset(2, undefined, undefined);
    }
    static constexpr uint32_t PreDataUpdate()
    {
        return PlatformOffset(15, undefined, undefined);
    }
    static constexpr uint32_t Paint()
    {
        return PlatformOffset(14, undefined, undefined);
    }
    static constexpr uint32_t SendDatagram()
    {
        return PlatformOffset(47, undefined, 47);
    }
    static constexpr uint32_t IsPlayingTimeDemo()
    {
        return PlatformOffset(8, undefined, 8);
    }
    static constexpr uint32_t RegisterFileWhitelist()
    {
        return PlatformOffset(94, undefined, undefined);
    }
    static constexpr uint32_t StartMessageMode()
    {
        return PlatformOffset(23, undefined, undefined);
    }
    static constexpr uint32_t StopMessageMode()
    {
        return PlatformOffset(24, undefined, undefined);
    }
    static constexpr uint32_t ChatPrintf()
    {
        return PlatformOffset(22, undefined, undefined);
    }
    static constexpr uint32_t ServerCmdKeyValues()
    {
        return PlatformOffset(128, undefined, undefined);
    }
    static constexpr uint32_t EmitSound1()
    {
        return PlatformOffset(4, undefined, undefined);
    }
    static constexpr uint32_t EmitSound2()
    {
        return PlatformOffset(5, undefined, undefined);
    }
    static constexpr uint32_t EmitSound3()
    {
        return PlatformOffset(6, undefined, undefined);
    }
    static constexpr uint32_t GetMaxItemCount()
    {
        return PlatformOffset(10, undefined, undefined);
    }
    static constexpr uint32_t RunCommand()
    {
        return PlatformOffset(18, undefined, undefined);
    }
    static constexpr uint32_t m_bUsingActionSlot()
    {
        return PlatformOffset(0x2fb8, undefined, undefined);
    }
};
