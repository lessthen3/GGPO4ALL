/* -----------------------------------------------------------------------
 * .net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "../include/backends/p2p.h"

namespace GGPO
{
     static constexpr int RECOMMENDATION_INTERVAL = 240;
     static constexpr int DEFAULT_DISCONNECT_TIMEOUT = 5000;
     static constexpr int DEFAULT_DISCONNECT_NOTIFY_START = 750;

     Peer2PeerBackend::Peer2PeerBackend
     (
         const char* gamename,
         uint16_t localport,
         int num_players,
         int input_size
     ) :
         _num_players(num_players),
         _input_size(input_size),
         _sync(_local_connect_status),
         _disconnect_timeout(DEFAULT_DISCONNECT_TIMEOUT),
         _disconnect_notify_start(DEFAULT_DISCONNECT_NOTIFY_START),
         _num_spectators(0),
         _next_spectator_frame(0)
     {
         _synchronizing = true;
         _next_recommended_sleep = 0;

         /*
          * Initialize the synchronziation layer
          */
         Sync::Config config = { 0 };
         config.num_players = num_players;
         config.input_size = input_size;
         config.num_prediction_frames = MAX_PREDICTION_FRAMES;
         _sync.Init(config);

         /*
          * Initialize the UDP port
          */
         _udp.Init(localport, &_poll, this);

         _endpoints = new UdpProtocol[_num_players];
         memset(_local_connect_status, 0, sizeof(_local_connect_status));

         for (int i = 0; i < _local_connect_status.size(); i++)
         {
             _local_connect_status[i].last_frame = -1;
         }

         /*
          * Preload the ROM
          */
          //_callbacks.begin_game(gamename); //THIS IS DEPRECATED APPARENTLY SO IDK WHAT ITS DOIN HERE
     }

     Peer2PeerBackend::~Peer2PeerBackend()
     {
         delete[] _endpoints;
     }

     bool 
         Peer2PeerBackend::InitializeLogger
         (
             const string& fp_DesiredOutputDirectory,
             const string& fp_DesiredLoggerName,
             const string& fp_MinLogLevel = "trace",
             const string& fp_MaxLogLevel = "fatal"
         )
     {
         logger = make_unique<LogManager>();
         logger->Initialize(fp_DesiredOutputDirectory, fp_DesiredLoggerName, fp_MinLogLevel, fp_MaxLogLevel);

         logger->LogAndPrint("Successfully initialized logger!", "p2p.cpp", LogManager::LogLevel::Info);
     }


     void
         Peer2PeerBackend::AddRemotePlayer
         (
             char* ip,
             uint16_t port,
             int queue
         )
     {
         /*
          * Start the state machine (xxx: no)
          */
         _synchronizing = true;

         _endpoints[queue].Init(&_udp, _poll, queue, ip, port, _local_connect_status);
         _endpoints[queue].SetDisconnectTimeout(_disconnect_timeout);
         _endpoints[queue].SetDisconnectNotifyStart(_disconnect_notify_start);
         _endpoints[queue].Synchronize();
     }

     ErrorCode Peer2PeerBackend::AddSpectator(char* ip,
         uint16_t port)
     {
         if (_num_spectators == MAX_SPECTATORS)
         {
             return ErrorCode::TOO_MANY_SPECTATORS;
         }
         /*
          * Currently, we can only add spectators before the game starts.
          */
         if (not _synchronizing)
         {
             return ErrorCode::INVALID_REQUEST;
         }

         int queue = _num_spectators++;

         _spectators[queue].Init(&_udp, _poll, queue + 1000, ip, port, _local_connect_status);
         _spectators[queue].SetDisconnectTimeout(_disconnect_timeout);
         _spectators[queue].SetDisconnectNotifyStart(_disconnect_notify_start);
         _spectators[queue].Synchronize();

         return ErrorCode::OK;
     }

     ErrorCode
         Peer2PeerBackend::DoPoll(const int fp_Timeout)
     {
         if (not _sync.InRollback())
         {
             _poll.Pump(0);

             PollUdpProtocolEvents();

             if (not _synchronizing)
             {
                 _sync.CheckSimulation(fp_Timeout);

                 // notify all of our endpoints of their local frame number for their
                 // next connection quality report
                 int current_frame = _sync.GetFrameCount();

                 for (int i = 0; i < _num_players; i++)
                 {
                     _endpoints[i].SetLocalFrameNumber(current_frame);
                 }

                 int total_min_confirmed;
                 if (_num_players <= 2)
                 {
                     total_min_confirmed = Poll2Players(current_frame);
                 }
                 else
                 {
                     total_min_confirmed = PollNPlayers(current_frame);
                 }

                 logger->LogAndPrint(format("last confirmed frame in p2p backend is {}.", total_min_confirmed), "p2p.cpp", LogManager::LogLevel::Info);

                 if (total_min_confirmed >= 0)
                 {
                     ASSERT(total_min_confirmed != INT_MAX);

                     if (_num_spectators > 0)
                     {
                         while (_next_spectator_frame <= total_min_confirmed)
                         {
                             logger->LogAndPrint(format("pushing frame {} to spectators.", _next_spectator_frame), "p2p.cpp", LogManager::LogLevel::Info);

                             GameInput input;
                             input.frame = _next_spectator_frame;
                             input.size = _input_size * _num_players;

                             _sync.GetConfirmedInputs(input.bits, _input_size * _num_players, _next_spectator_frame);

                             for (int i = 0; i < _num_spectators; i++)
                             {
                                 _spectators[i].SendInput(input);
                             }
                             _next_spectator_frame++;
                         }
                     }
                     logger->LogAndPrint(format("setting confirmed frame in sync to {}.", total_min_confirmed), "p2p.cpp", LogManager::LogLevel::Info);

                     _sync.SetLastConfirmedFrame(total_min_confirmed);
                 }

                 // send timesync notifications if now is the proper time
                 if (current_frame > _next_recommended_sleep)
                 {
                     int interval = 0;

                     for (int i = 0; i < _num_players; i++)
                     {
                         interval = MAX(interval, _endpoints[i].RecommendFrameDelay());
                     }

                     if (interval > 0)
                     {
                         Event info;
                         info.code = EventCode::TimeSync;
                         info.u.timesync.frames_ahead = interval;
                         _callbacks.on_event(&info);
                         _next_recommended_sleep = current_frame + RECOMMENDATION_INTERVAL;
                     }
                 }
             }
         }
         return ErrorCode::OK;
     }

     int Peer2PeerBackend::Poll2Players(int current_frame)
     {
         int i;

         // discard confirmed frames as appropriate
         int total_min_confirmed = MAX_INT;
         for (i = 0; i < _num_players; i++)
         {
             bool queue_connected = true;

             if (_endpoints[i].IsRunning())
             {
                 int ignore;
                 queue_connected = _endpoints[i].GetPeerConnectStatus(i, &ignore);
             }
             if (not _local_connect_status[i].disconnected)
             {
                 total_min_confirmed = MIN(_local_connect_status[i].last_frame, total_min_confirmed);
             }

             logger->LogAndPrint(format("  local endp: connected = {}, last_received = {}, total_min_confirmed = {}.", not _local_connect_status[i].disconnected, _local_connect_status[i].last_frame, total_min_confirmed), "p2p.cpp", LogManager::LogLevel::Info);

             if (not queue_connected && not _local_connect_status[i].disconnected)
             {
                 logger->LogAndPrint(format("disconnecting i {} by remote request.", i), "p2p.cpp", LogManager::LogLevel::Info);
                 DisconnectPlayerQueue(i, total_min_confirmed);
             }

             logger->LogAndPrint(format("  total_min_confirmed = {}.", total_min_confirmed), "p2p.cpp", LogManager::LogLevel::Info);
         }
         return total_min_confirmed;
     }

     int Peer2PeerBackend::PollNPlayers(int current_frame)
     {
         int i, queue, last_received;

         // discard confirmed frames as appropriate
         int total_min_confirmed = MAX_INT;

         for (queue = 0; queue < _num_players; queue++)
         {
             bool queue_connected = true;
             int queue_min_confirmed = MAX_INT;

             logger->LogAndPrint(format("considering queue {}.", queue), "p2p.cpp", LogManager::LogLevel::Info);

             for (i = 0; i < _num_players; i++)
             {
                 // we're going to do a lot of logic here in consideration of endpoint i.
                 // keep accumulating the minimum confirmed point for all n*n packets and
                 // throw away the rest.
                 if (_endpoints[i].IsRunning())
                 {
                     bool connected = _endpoints[i].GetPeerConnectStatus(queue, &last_received);

                     queue_connected = queue_connected && connected;
                     queue_min_confirmed = MIN(last_received, queue_min_confirmed);
                     logger->LogAndPrint(format("  endpoint {}: connected = {}, last_received = {}, queue_min_confirmed = {}.", i, connected, last_received, queue_min_confirmed), "p2p.cpp", LogManager::LogLevel::Info);
                 }
                 else
                 {
                     logger->LogAndPrint(format("  endpoint {}: ignoring... not running.", i), "p2p.cpp", LogManager::LogLevel::Info);
                 }
             }
             // merge in our local status only if we're still connected!
             if (not _local_connect_status[queue].disconnected)
             {
                 queue_min_confirmed = MIN(_local_connect_status[queue].last_frame, queue_min_confirmed);
             }

             logger->LogAndPrint(format("  local endp: connected = {}, last_received = {}, queue_min_confirmed = {}.", not _local_connect_status[queue].disconnected, _local_connect_status[queue].last_frame, queue_min_confirmed), "p2p.cpp", LogManager::LogLevel::Info);

             if (queue_connected)
             {
                 total_min_confirmed = MIN(queue_min_confirmed, total_min_confirmed);
             }
             else
             {
                 // check to see if this disconnect notification is further back than we've been before.  If
                 // so, we need to re-adjust.  This can happen when we detect our own disconnect at frame n
                 // and later receive a disconnect notification for frame n-1.
                 if (not _local_connect_status[queue].disconnected or _local_connect_status[queue].last_frame > queue_min_confirmed)
                 {
                     logger->LogAndPrint(format("disconnecting queue {} by remote request.", queue), "p2p.cpp", LogManager::LogLevel::Info);
                     DisconnectPlayerQueue(queue, queue_min_confirmed);
                 }
             }
             logger->LogAndPrint(format("  total_min_confirmed = {}.", total_min_confirmed), "p2p.cpp", LogManager::LogLevel::Info);
         }
         return total_min_confirmed;
     }


     ErrorCode
         Peer2PeerBackend::AddPlayer
         (
             Player* player,
             PlayerHandle* handle
         )
     {
         if (player->type == PlayerType::Spectator)
         {
             return AddSpectator(player->u.remote.ip_address, player->u.remote.port);
         }

         int queue = player->player_num - 1;

         if (player->player_num < 1 || player->player_num > _num_players)
         {
             return ErrorCode::PLAYER_OUT_OF_RANGE;
         }

         *handle = QueueToPlayerHandle(queue);

         if (player->type == PlayerType::Remote)
         {
             AddRemotePlayer(player->u.remote.ip_address, player->u.remote.port, queue);
         }
         return ErrorCode::OK;
     }

     ErrorCode
         Peer2PeerBackend::AddLocalInput
         (
             PlayerHandle player,
             void* values,
             int size
         )
     {
         int queue;
         GameInput input;
         ErrorCode result;

         if (_sync.InRollback())
         {
             return ErrorCode::IN_ROLLBACK;
         }
         if (_synchronizing)
         {
             return ErrorCode::NOT_SYNCHRONIZED;
         }

         result = PlayerHandleToQueue(player, &queue);

         if (not Succeeded(result))
         {
             return result;
         }

         input.init(-1, (char*)values, size);

         // Feed the input for the current frame into the synchronzation layer.
         if (not _sync.AddLocalInput(queue, input))
         {
             return ErrorCode::PREDICTION_THRESHOLD;
         }

         if (input.frame != GameInput::NullFrame)
         {  // xxx: <- comment why this is the case
            // Update the local connect status state to indicate that we've got a
            // confirmed local frame for this player.  this must come first so it
            // gets incorporated into the next packet we send.

             logger->LogAndPrint(format("setting local connect status for local queue {} to {}", queue, input.frame), "p2p.cpp", LogManager::LogLevel::Info);
             _local_connect_status[queue].last_frame = input.frame;

             // Send the input to all the remote players.
             for (int i = 0; i < _num_players; i++)
             {
                 if (_endpoints[i].IsInitialized())
                 {
                     _endpoints[i].SendInput(input);
                 }
             }
         }

         return ErrorCode::OK;
     }


     ErrorCode
         Peer2PeerBackend::SyncInput
         (
             void* values,
             int size,
             int* disconnect_flags
         )
     {
         int flags;

         // Wait until we've started to return inputs.
         if (_synchronizing)
         {
             return ErrorCode::NOT_SYNCHRONIZED;
         }
         flags = _sync.SynchronizeInputs(values, size);

         if (disconnect_flags)
         {
             *disconnect_flags = flags;
         }

         return ErrorCode::OK;
     }

     ErrorCode
         Peer2PeerBackend::IncrementFrame(void)
     {
         logger->LogAndPrint(format("End of frame ({})...", _sync.GetFrameCount()), "p2p.cpp", LogManager::LogLevel::Info);
         _sync.IncrementFrame();
         DoPoll(0);
         PollSyncEvents();

         return ErrorCode::OK;
     }


     void
         Peer2PeerBackend::PollSyncEvents(void)
     {
         Sync::Event e;
         while (_sync.GetEvent(e)) {
             OnSyncEvent(e);
         }
         return;
     }

     void
         Peer2PeerBackend::PollUdpProtocolEvents(void)
     {
         UdpProtocol::Event evt;

         for (int i = 0; i < _num_players; i++)
         {
             while (_endpoints[i].GetEvent(evt))
             {
                 OnUdpProtocolPeerEvent(evt, i);
             }
         }
         for (int i = 0; i < _num_spectators; i++)
         {
             while (_spectators[i].GetEvent(evt))
             {
                 OnUdpProtocolSpectatorEvent(evt, i);
             }
         }
     }

     void
         Peer2PeerBackend::OnUdpProtocolPeerEvent(UdpProtocol::Event& evt, int queue)
     {
         OnUdpProtocolEvent(evt, QueueToPlayerHandle(queue));

         switch (evt.type)
         {
         case UdpProtocol::Event::Input:
             if (not _local_connect_status[queue].disconnected)
             {
                 int current_remote_frame = _local_connect_status[queue].last_frame;
                 int new_remote_frame = evt.u.input.input.frame;
                 ASSERT(current_remote_frame == -1 || new_remote_frame == (current_remote_frame + 1));

                 _sync.AddRemoteInput(queue, evt.u.input.input);
                 // Notify the other endpoints which frame we received from a peer
                 logger->LogAndPrint(format("setting remote connect status for queue {} to {}", queue, evt.u.input.input.frame), "p2p.cpp", LogManager::LogLevel::Info);
                 _local_connect_status[queue].last_frame = evt.u.input.input.frame;
             }
             break;

         case UdpProtocol::Event::Disconnected:
             DisconnectPlayer(QueueToPlayerHandle(queue));
             break;
         }
     }


     void
         Peer2PeerBackend::OnUdpProtocolSpectatorEvent(UdpProtocol::Event& evt, int queue)
     {
         PlayerHandle handle = QueueToSpectatorHandle(queue);
         OnUdpProtocolEvent(evt, handle);

         Event info;

         switch (evt.type) //why tf is this a switch statement LMFAO
         {
         case UdpProtocol::Event::Disconnected:
             _spectators[queue].Disconnect();

             info.code = EventCode::DisconnectedFromPeer;
             info.u.disconnected.player = handle;
             _callbacks.on_event(&info);

             break;
         }
     }

     void
         Peer2PeerBackend::OnUdpProtocolEvent(UdpProtocol::Event& evt, PlayerHandle handle)
     {
         Event info;

         switch (evt.type)
         {
         case UdpProtocol::Event::Connected:
             info.code = EventCode::ConnectedToPeer;
             info.u.connected.player = handle;
             _callbacks.on_event(&info);
             break;

         case UdpProtocol::Event::Synchronizing:
             info.code = EventCode::SynchronizingWithPeer;
             info.u.synchronizing.player = handle;
             info.u.synchronizing.count = evt.u.synchronizing.count;
             info.u.synchronizing.total = evt.u.synchronizing.total;
             _callbacks.on_event(&info);
             break;

         case UdpProtocol::Event::Synchronzied:
             info.code = EventCode::SynchronizedWithPeer;
             info.u.synchronized.player = handle;
             _callbacks.on_event(&info);

             CheckInitialSync();
             break;

         case UdpProtocol::Event::NetworkInterrupted:
             info.code = EventCode::ConnectionInterrupted;
             info.u.connection_interrupted.player = handle;
             info.u.connection_interrupted.disconnect_timeout = evt.u.network_interrupted.disconnect_timeout;
             _callbacks.on_event(&info);
             break;

         case UdpProtocol::Event::NetworkResumed:
             info.code = EventCode::ConnectionResumed;
             info.u.connection_resumed.player = handle;
             _callbacks.on_event(&info);
             break;
         }
     }

     /*
      * Called only as the result of a local decision to disconnect.  The remote
      * decisions to disconnect are a result of us parsing the peer_connect_settings
      * blob in every endpoint periodically.
      */
     ErrorCode
         Peer2PeerBackend::DisconnectPlayer(PlayerHandle player)
     {
         int queue;
         ErrorCode result;

         result = PlayerHandleToQueue(player, &queue);

         if (not Succeeded(result))
         {
             return result;
         }

         if (_local_connect_status[queue].disconnected)
         {
             return ErrorCode::PLAYER_DISCONNECTED;
         }

         if (not _endpoints[queue].IsInitialized())
         {
             int current_frame = _sync.GetFrameCount();
             // xxx: we should be tracking who the local player is, but for now assume
             // that if the endpoint is not initalized, this must be the local player.
             logger->LogAndPrint(format("Disconnecting local player {} at frame {} by user request.", queue, _local_connect_status[queue].last_frame), "p2p.cpp", LogManager::LogLevel::Info);
             for (int i = 0; i < _num_players; i++)
             {
                 if (_endpoints[i].IsInitialized())
                 {
                     DisconnectPlayerQueue(i, current_frame);
                 }
             }
         }
         else
         {
             logger->LogAndPrint(format("Disconnecting queue {} at frame {} by user request.", queue, _local_connect_status[queue].last_frame), "p2p.cpp", LogManager::LogLevel::Info);
             DisconnectPlayerQueue(queue, _local_connect_status[queue].last_frame);
         }
         return ErrorCode::OK;
     }

     void
         Peer2PeerBackend::DisconnectPlayerQueue(int queue, int syncto)
     {
         Event info;
         int framecount = _sync.GetFrameCount();

         _endpoints[queue].Disconnect();

         logger->LogAndPrint(format("Changing queue {} local connect status for last frame from {} to {} on disconnect request (current: {}).", queue, _local_connect_status[queue].last_frame, syncto, framecount), "p2p.cpp", LogManager::LogLevel::Info);

         _local_connect_status[queue].disconnected = 1;
         _local_connect_status[queue].last_frame = syncto;

         if (syncto < framecount)
         {
             logger->LogAndPrint(format("adjusting simulation to account for the fact that {} disconnected @ {}.", queue, syncto), "p2p.cpp", LogManager::LogLevel::Info);
             _sync.AdjustSimulation(syncto);
             logger->LogAndPrint("finished adjusting simulation.", "p2p.cpp", LogManager::LogLevel::Info);
         }

         info.code = EventCode::DisconnectedFromPeer;
         info.u.disconnected.player = QueueToPlayerHandle(queue);
         _callbacks.on_event(&info);

         CheckInitialSync();
     }


     ErrorCode
         Peer2PeerBackend::GetNetworkStats(NetworkStats* stats, PlayerHandle player)
     {
         int queue;
         ErrorCode result;

         result = PlayerHandleToQueue(player, &queue);

         if (not Succeeded(result))
         {
             return result;
         }

         memset(stats, 0, sizeof * stats);
         _endpoints[queue].GetNetworkStats(stats);

         return ErrorCode::OK;
     }

     ErrorCode
         Peer2PeerBackend::SetFrameDelay(PlayerHandle player, int delay)
     {
         int queue;
         ErrorCode result;

         result = PlayerHandleToQueue(player, &queue);

         if (not Succeeded(result))
         {
             return result;
         }
         _sync.SetFrameDelay(queue, delay);
         return ErrorCode::OK;
     }

     ErrorCode
         Peer2PeerBackend::SetDisconnectTimeout(int timeout)
     {
         _disconnect_timeout = timeout;
         for (int i = 0; i < _num_players; i++)
         {
             if (_endpoints[i].IsInitialized())
             {
                 _endpoints[i].SetDisconnectTimeout(_disconnect_timeout);
             }
         }
         return ErrorCode::OK;
     }

     ErrorCode
         Peer2PeerBackend::SetDisconnectNotifyStart(int timeout)
     {
         _disconnect_notify_start = timeout;

         for (int i = 0; i < _num_players; i++)
         {
             if (_endpoints[i].IsInitialized())
             {
                 _endpoints[i].SetDisconnectNotifyStart(_disconnect_notify_start);
             }
         }
         return ErrorCode::OK;
     }

     ErrorCode
         Peer2PeerBackend::PlayerHandleToQueue(PlayerHandle player, int* queue)
     {
         int offset = ((int)player - 1);

         if (offset < 0 or offset >= _num_players)
         {
             return ErrorCode::INVALID_PLAYER_HANDLE;
         }

         *queue = offset;

         return ErrorCode::OK;
     }


     void
         Peer2PeerBackend::OnMsg(sockaddr_in& from, UdpMsg* msg, int len)
     {
         for (int i = 0; i < _num_players; i++)
         {
             if (_endpoints[i].HandlesMsg(from, msg))
             {
                 _endpoints[i].OnMsg(msg, len);
                 return;
             }
         }
         for (int i = 0; i < _num_spectators; i++)
         {
             if (_spectators[i].HandlesMsg(from, msg))
             {
                 _spectators[i].OnMsg(msg, len);
                 return;
             }
         }
     }

     void
         Peer2PeerBackend::CheckInitialSync()
     {
         int i; //umm okay ig lmfao

         if (_synchronizing)
         {
             // Check to see if everyone is now synchronized.  If so,
             // go ahead and tell the client that we're ok to accept input.
             for (i = 0; i < _num_players; i++)
             {
                 // xxx: IsInitialized() must go... we're actually using it as a proxy for "represents the local player"
                 if (_endpoints[i].IsInitialized() and not _endpoints[i].IsSynchronized() and not _local_connect_status[i].disconnected)
                 {
                     return;
                 }
             }
             for (i = 0; i < _num_spectators; i++)
             {
                 if (_spectators[i].IsInitialized() and not _spectators[i].IsSynchronized())
                 {
                     return;
                 }
             }

             Event info;
             info.code = EventCode::Running;
             _callbacks.on_event(&info);
             _synchronizing = false;
         }
     }
}
