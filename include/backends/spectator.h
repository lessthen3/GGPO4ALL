/* -----------------------------------------------------------------------
 * .net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#pragma once

#include "platform_common.h"
#include "poll.h"
#include "sync.h"
#include "Session.h"
#include "timesync.h"
#include "network/udp_proto.h"

namespace GGPO
{
	constexpr int SPECTATOR_FRAME_BUFFER_SIZE = 64;

	 class SpectatorBackend : public Session, IPollSink, Udp::Callbacks
	 {
	 public:
		 SpectatorBackend(const char* gamename, uint16_t localport, int num_players, int input_size, char* hostip, uint16_t hostport);
		 virtual ~SpectatorBackend();


	 public:
		 virtual ErrorCode DoPoll(int timeout);
		 virtual ErrorCode SyncInput(void* values, int size, int* disconnect_flags);
		 virtual ErrorCode IncrementFrame(void);

		 virtual ErrorCode AddPlayer(Player* player, PlayerHandle* handle)
		 {
			 return ErrorCode::UNSUPPORTED;
		 }

		 virtual ErrorCode AddLocalInput(PlayerHandle player, void* values, int size) //??????
		 {
			 return ErrorCode::OK;
		 }

		 virtual ErrorCode DisconnectPlayer(PlayerHandle handle)
		 {
			 return ErrorCode::UNSUPPORTED;
		 }

		 virtual ErrorCode GetNetworkStats(NetworkStats* stats, PlayerHandle handle)
		 {
			 return ErrorCode::UNSUPPORTED;
		 }

		 virtual ErrorCode SetFrameDelay(PlayerHandle player, int delay)
		 {
			 return ErrorCode::UNSUPPORTED;
		 }

		 virtual ErrorCode SetDisconnectTimeout(int timeout)
		 {
			 return ErrorCode::UNSUPPORTED;
		 }

		 virtual ErrorCode SetDisconnectNotifyStart(int timeout)
		 {
			 return ErrorCode::UNSUPPORTED;
		 }

	 public:
		 virtual void OnMsg(sockaddr_in& from, UdpMsg* msg, int len);

	 protected:
		 void PollUdpProtocolEvents(void);
		 void CheckInitialSync(void); //?????????

		 void OnUdpProtocolEvent(UdpProtocol::Event& e);

	 protected:
		 Poll                  _poll;
		 Udp                   _udp;
		 UdpProtocol           _host;
		 bool                  _synchronizing;
		 int                   _input_size;
		 int                   _num_players;
		 int                   _next_input_to_send;
		 GameInput             _inputs[SPECTATOR_FRAME_BUFFER_SIZE];
	 };
}
