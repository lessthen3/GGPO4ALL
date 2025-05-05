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
	 class Peer2PeerBackend : public Session, IPollSink, Udp::Callbacks
	 {
	 public:
		 Peer2PeerBackend(const char* gamename, uint16_t localport, int num_players, int input_size);
		 virtual ~Peer2PeerBackend();


	 public:
		 virtual ErrorCode DoPoll(int timeout);
		 virtual ErrorCode AddPlayer(Player* player, PlayerHandle* handle);
		 virtual ErrorCode AddLocalInput(PlayerHandle player, void* values, int size);
		 virtual ErrorCode SyncInput(void* values, int size, int* disconnect_flags);
		 virtual ErrorCode IncrementFrame(void);
		 virtual ErrorCode DisconnectPlayer(PlayerHandle handle);
		 virtual ErrorCode GetNetworkStats(NetworkStats* stats, PlayerHandle handle);
		 virtual ErrorCode SetFrameDelay(PlayerHandle player, int delay);
		 virtual ErrorCode SetDisconnectTimeout(int timeout);
		 virtual ErrorCode SetDisconnectNotifyStart(int timeout);

		 virtual bool 
			 InitializeLogger
			 (
				 const string& fp_DesiredOutputDirectory, 
				 const string& fp_DesiredLoggerName,
				 const string& fp_MinLogLevel = "trace",
				 const string& fp_MaxLogLevel = "fatal"
			 );

	 public:
		 virtual void OnMsg(sockaddr_in& from, UdpMsg* msg, int len);

	 protected:
		 ErrorCode PlayerHandleToQueue(PlayerHandle player, int* queue);
		 PlayerHandle QueueToPlayerHandle(int queue) { return (PlayerHandle)(queue + 1); }
		 PlayerHandle QueueToSpectatorHandle(int queue) { return (PlayerHandle)(queue + 1000); } /* out of range of the player array, basically */
		 void DisconnectPlayerQueue(int queue, int syncto);
		 void PollSyncEvents(void);
		 void PollUdpProtocolEvents(void);
		 void CheckInitialSync(void);
		 int Poll2Players(int current_frame);
		 int PollNPlayers(int current_frame);
		 void AddRemotePlayer(char* remoteip, uint16_t reportport, int queue);
		 ErrorCode AddSpectator(char* remoteip, uint16_t reportport);
		 virtual void OnSyncEvent(Sync::Event& e) { }
		 virtual void OnUdpProtocolEvent(UdpProtocol::Event& e, PlayerHandle handle);
		 virtual void OnUdpProtocolPeerEvent(UdpProtocol::Event& e, int queue);
		 virtual void OnUdpProtocolSpectatorEvent(UdpProtocol::Event& e, int queue);

	 protected:
		 Poll                  _poll;
		 Sync                  _sync;
		 Udp                   _udp;
		 UdpProtocol* _endpoints;
		 array <UdpProtocol, MAX_SPECTATORS> _spectators = {};
		 int                   _num_spectators;
		 int                   _input_size;

		 bool                  _synchronizing;
		 int                   _num_players;
		 int                   _next_recommended_sleep;

		 int                   _next_spectator_frame;
		 int                   _disconnect_timeout;
		 int                   _disconnect_notify_start;

		 array<UdpMsg::connect_status, UDP_MSG_MAX_PLAYERS> _local_connect_status = {};
	 };
}
