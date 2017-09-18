/*
 * offsets.hpp
 *
 *  Created on: May 4, 2017
 *      Author: nullifiedcat
 */

#ifndef OFFSETS_HPP_
#define OFFSETS_HPP_

#include <stdint.h>
//#include <exception>

struct offsets {

	static constexpr uint32_t GetUserCmd() { 			return 8; 	}
	static constexpr uint32_t ShouldDraw() { 			return 136; }
	static constexpr uint32_t DrawModelExecute() { 		return 19; 	}
	static constexpr uint32_t GetClientName() { 		return 44; 	}
	static constexpr uint32_t ProcessSetConVar() { 		return 4;	}
	static constexpr uint32_t ProcessGetCvarValue() { 	return 29; 	}
	static constexpr uint32_t GetFriendPersonaName() { 	return 7;	}
	static constexpr uint32_t CreateMove() { 			return 22; 	}
	static constexpr uint32_t PaintTraverse() { 		return 42;	}
	static constexpr uint32_t OverrideView() { 			return 17; 	}
	static constexpr uint32_t FrameStageNotify() { 		return 35; 	}
	static constexpr uint32_t DispatchUserMessage() { 	return 36; 	}
	static constexpr uint32_t CanPacket() { 			return 57; 	}
	static constexpr uint32_t SendNetMsg() { 			return 41; 	}
	static constexpr uint32_t Shutdown() { 				return 37; 	}
	static constexpr uint32_t IN_KeyEvent() { 			return 20; 	}
	static constexpr uint32_t HandleInputEvent() { 		return 78; 	}
	static constexpr uint32_t LevelInit() { 			return 23;	}
	static constexpr uint32_t LevelShutdown() { 		return 24; 	}
	static constexpr uint32_t BeginFrame() { 			return 5;	}
	static constexpr uint32_t FireGameEvent() {			return 2;	}

	static constexpr uint32_t lastoutgoingcommand() { 	return 191;	}
	static constexpr uint32_t m_nOutSequenceNr() { 		return 8;	}
	static constexpr uint32_t m_NetChannel() { 			return 19;	}

};

#endif /* OFFSETS_HPP_ */
