/* -----------------------------------------------------------------------
 * .net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#pragma once

#include "ggponet.h"
#include "platform_common.h"

namespace GGPO
{
	 struct Session
	 {
		 virtual ~Session() { }
		 virtual ErrorCode AddPlayer(Player* player, PlayerHandle* handle) = 0;
		 virtual ErrorCode AddLocalInput(PlayerHandle player, void* values, int size) = 0;
		 virtual ErrorCode SyncInput(void* values, int size, int* disconnect_flags) = 0;
		 virtual bool 
			 InitializeLogger
			(
				const string& fp_DesiredOutputDirectory, 
				const string& fp_DesiredLoggerName,
				const string& fp_MinLogLevel = "trace",
				const string& fp_MaxLogLevel = "fatal"
			) = 0;

		 virtual ErrorCode DoPoll(int timeout) { return ErrorCode::OK; }
		 virtual ErrorCode IncrementFrame(void) { return ErrorCode::OK; }
		 virtual ErrorCode Chat(char* text) { return ErrorCode::OK; }
		 virtual ErrorCode DisconnectPlayer(PlayerHandle handle) { return ErrorCode::OK; }
		 virtual ErrorCode GetNetworkStats(NetworkStats* stats, PlayerHandle handle) { return ErrorCode::OK; }

		 virtual ErrorCode SetFrameDelay(PlayerHandle player, int delay) { return ErrorCode::UNSUPPORTED; }
		 virtual ErrorCode SetDisconnectTimeout(int timeout) { return ErrorCode::UNSUPPORTED; }
		 virtual ErrorCode SetDisconnectNotifyStart(int timeout) { return ErrorCode::UNSUPPORTED; }


	 protected:
		 unique_ptr<LogManager> logger = nullptr;
	 };
}

