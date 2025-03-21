/* -----------------------------------------------------------------------
 * .net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#pragma once

#include <stdarg.h>
#include <string_view>

namespace GGPO
{
     constexpr int MAX_PLAYERS = 4;
     constexpr int MAX_PREDICTION_FRAMES = 8;
     constexpr int MAX_SPECTATORS = 32;

     constexpr int SPECTATOR_INPUT_INTERVAL = 4;

     typedef struct Session Session;

     typedef int PlayerHandle;

     enum class PlayerType : int
     {
         Local,
         Remote,
         Spectator,
     };

     /*
      * The Player structure used to describe players in ggpo_add_player
      *
      * size: Should be set to the sizeof(Player)
      *
      * type: One of the PlayerType values describing how inputs should be handled
      *       Local players must have their inputs updated every frame via
      *       ggpo_add_local_inputs.  Remote players values will come over the
      *       network.
      *
      * player_num: The player number.  Should be between 1 and the number of players
      *       In the game (e.g. in a 2 player game, either 1 or 2).
      *
      * If type == PLAYERTYPE_REMOTE:
      *
      * u.remote.ip_address:  The ip address of the ggpo session which will host this
      *       player.
      *
      * u.remote.port: The port where udp packets should be sent to reach this player.
      *       All the local inputs for this session will be sent to this player at
      *       ip_address:port.
      *
      */

     struct Player
     {
         int               size;
         PlayerType    type;
         int               player_num;

         union
         {
             struct
             {
             } local;
             struct
             {
                 char           ip_address[32];
                 unsigned short port;
             } remote;
         } u;
     };

     struct LocalEndpoint
     {
         int      player_num;
     };


     enum class ErrorCode : int
     {
         OK = 69,
         SUCCESS = 0,
         GENERAL_FAILURE = -1,
         INVALID_SESSION = 1,
         INVALID_PLAYER_HANDLE = 2,
         PLAYER_OUT_OF_RANGE = 3,
         PREDICTION_THRESHOLD = 4,
         UNSUPPORTED = 5,
         NOT_SYNCHRONIZED = 6,
         IN_ROLLBACK = 7,
         INPUT_DROPPED = 8,
         PLAYER_DISCONNECTED = 9,
         TOO_MANY_SPECTATORS = 10,
         INVALID_REQUEST = 11,
         FATAL_DESYNC = 12
     };


     consteval string_view
         ErrorToString(ErrorCode fp_ErrorCode)
     {
         switch (fp_ErrorCode)
         {
         case ErrorCode::OK: return "No error.";
         case ErrorCode::GENERAL_FAILURE: return "General failure.";
         case ErrorCode::INVALID_SESSION: return "Invalid session.";
         case ErrorCode::INVALID_PLAYER_HANDLE: return "Invalid player handle.";
         case ErrorCode::PLAYER_OUT_OF_RANGE: return "Player out of range.";
         case ErrorCode::PREDICTION_THRESHOLD: return "Prediction threshold exceeded.";
         case ErrorCode::UNSUPPORTED: return "Unsupported operation.";
         case ErrorCode::NOT_SYNCHRONIZED: return "Not synchronized.";
         case ErrorCode::IN_ROLLBACK: return "Currently in rollback.";
         case ErrorCode::INPUT_DROPPED: return "Input was dropped.";
         case ErrorCode::PLAYER_DISCONNECTED: return "Player disconnected.";
         case ErrorCode::TOO_MANY_SPECTATORS: return "Too many spectators connected.";
         case ErrorCode::INVALID_REQUEST: return "Invalid request.";
         case ErrorCode::FATAL_DESYNC: return "Fatal desynchronization detected!";
         default: return "Unknown  error.";
         }
     }

     constexpr bool
         Succeeded(ErrorCode fp_Result)
         noexcept
     {
         return fp_Result == ErrorCode::SUCCESS;
     }

     constexpr int INVALID_HANDLE = -1;


     /*
      * The EventCode enumeration describes what type of event just happened.
      *
      * EVENTCODE_CONNECTED_TO_PEER - Handshake with the game running on the
      * other side of the network has been completed.
      *
      * EVENTCODE_SYNCHRONIZING_WITH_PEER - Beginning the synchronization
      * process with the client on the other end of the networking.  The count
      * and total fields in the u.synchronizing struct of the Event
      * object indicate progress.
      *
      * EVENTCODE_SYNCHRONIZED_WITH_PEER - The synchronziation with this
      * peer has finished.
      *
      * EVENTCODE_RUNNING - All the clients have synchronized.  You may begin
      * sending inputs with ggpo_synchronize_inputs.
      *
      * EVENTCODE_DISCONNECTED_FROM_PEER - The network connection on
      * the other end of the network has closed.
      *
      * EVENTCODE_TIMESYNC - The time synchronziation code has determined
      * that this client is too far ahead of the other one and should slow
      * down to ensure fairness.  The u.timesync.frames_ahead parameter in
      * the Event object indicates how many frames the client is.
      *
      */
     enum class EventCode : int
     {
         ConnectedToPeer = 1000,
         SynchronizingWithPeer = 1001,
         SynchronizedWithPeer = 1002,
         Running = 1003,
         DisconnectedFromPeer = 1004,
         TimeSync = 1005,
         ConnectionInterrupted = 1006,
         ConnectionResumed = 1007
     };

     consteval string_view
         EventToString(EventCode fp_EventCode)
     {
         switch (fp_EventCode)
         {
         case EventCode::ConnectedToPeer:        return "ConnectedToPeer";
         case EventCode::SynchronizingWithPeer:  return "SynchronizingWithPeer";
         case EventCode::SynchronizedWithPeer:  return "SynchronizedWithPeer";
         case EventCode::Running:                   return "Running";
         case EventCode::DisconnectedFromPeer:  return "DisconnectedFromPeer";
         case EventCode::TimeSync:                return "TimeSync";
         case EventCode::ConnectionInterrupted:  return "ConnectionInterrupted";
         case EventCode::ConnectionResumed:      return "ConnectionResumed";
         default:                                            return "UnknownEvent";
         }
     }

     /*
      * The Event structure contains an asynchronous event notification sent
      * by the on_event callback.  See EventCode, above, for a detailed
      * explanation of each event.
      */
     struct Event
     {
         Event() = default;

         EventCode code;

         union
         {
             struct
             {
                 PlayerHandle  player;
             }connected;
             struct
             {
                 PlayerHandle  player;
                 int               count;
                 int               total;
             }synchronizing;
             struct
             {
                 PlayerHandle  player;
             }synchronized;
             struct
             {
                 PlayerHandle  player;
             }disconnected;
             struct
             {
                 int               frames_ahead;
             }timesync;
             struct
             {
                 PlayerHandle  player;
                 int               disconnect_timeout;
             }connection_interrupted;
             struct
             {
                 PlayerHandle  player;
             }connection_resumed;
         } u;
     };

     /*
      * The NetworkStats function contains some statistics about the current
      * session.
      *
      * network.send_queue_len - The length of the queue containing UDP packets
      * which have not yet been acknowledged by the end client.  The length of
      * the send queue is a rough indication of the quality of the connection.
      * The longer the send queue, the higher the round-trip time between the
      * clients.  The send queue will also be longer than usual during high
      * packet loss situations.
      *
      * network.recv_queue_len - The number of inputs currently buffered by the
      * .net network layer which have yet to be validated.  The length of
      * the prediction queue is roughly equal to the current frame number
      * minus the frame number of the last packet in the remote queue.
      *
      * network.ping - The roundtrip packet transmission time as calcuated
      * by .net.  This will be roughly equal to the actual round trip
      * packet transmission time + 2 the interval at which you call ggpo_idle
      * or ggpo_advance_frame.
      *
      * network.kbps_sent - The estimated bandwidth used between the two
      * clients, in kilobits per second.
      *
      * timesync.local_frames_behind - The number of frames .net calculates
      * that the local client is behind the remote client at this instant in
      * time.  For example, if at this instant the current game client is running
      * frame 1002 and the remote game client is running frame 1009, this value
      * will mostly likely roughly equal 7.
      *
      * timesync.remote_frames_behind - The same as local_frames_behind, but
      * calculated from the perspective of the remote player.
      *
      */
     struct NetworkStats
     {
         struct
         {
             int   send_queue_len;
             int   recv_queue_len;
             int   ping;
             int   kbps_sent;
         } network;
         struct
         {
             int   local_frames_behind;
             int   remote_frames_behind;
         } timesync;
     };

     /*
      * ggpo_start_session --
      *
      * Used to being a new .net session.  The ggpo object returned by ggpo_start_session
      * uniquely identifies the state for this session and should be passed to all other
      * functions.
      *
      * session - An out parameter to the new ggpo session object.
      *
      * cb - A SessionCallbacks structure which contains the callbacks you implement
      * to help .net synchronize the two games.  You must implement all functions in
      * cb, even if they do nothing but 'return true';
      *
      * game - The name of the game.  This is used internally for  for logging purposes only.
      *
      * num_players - The number of players which will be in this game.  The number of players
      * per session is fixed.  If you need to change the number of players or any player
      * disconnects, you must start a new session.
      *
      * input_size - The size of the game inputs which will be passsed to ggpo_add_local_input.
      *
      * local_port - The port  should bind to for UDP traffic.
      */
     ErrorCode
         ggpo_start_session
         (
             Session** session,
             const char* game,
             int num_players,
             int input_size,
             unsigned short localport
         );


     /*
      * ggpo_add_player --
      *
      * Must be called for each player in the session (e.g. in a 3 player session, must
      * be called 3 times).
      *
      * player - A Player struct used to describe the player.
      *
      * handle - An out parameter to a handle used to identify this player in the future.
      * (e.g. in the on_event callbacks).
      */
     ErrorCode
         ggpo_add_player
         (
             Session* session,
             Player* player,
             PlayerHandle* handle
         );


     /*
      * ggpo_start_synctest --
      *
      * Used to being a new .net sync test session.  During a sync test, every
      * frame of execution is run twice: once in prediction mode and once again to
      * verify the result of the prediction.  If the checksums of your save states
      * do not match, the test is aborted.
      *
      * cb - A SessionCallbacks structure which contains the callbacks you implement
      * to help .net synchronize the two games.  You must implement all functions in
      * cb, even if they do nothing but 'return true';
      *
      * game - The name of the game.  This is used internally for  for logging purposes only.
      *
      * num_players - The number of players which will be in this game.  The number of players
      * per session is fixed.  If you need to change the number of players or any player
      * disconnects, you must start a new session.
      *
      * input_size - The size of the game inputs which will be passsed to ggpo_add_local_input.
      *
      * frames - The number of frames to run before verifying the prediction.  The
      * recommended value is 1.
      *
      */
     ErrorCode
         ggpo_start_synctest
         (
             Session** session,
             char* game,
             int num_players,
             int input_size,
             int frames
         );


     /*
      * ggpo_start_spectating --
      *
      * Start a spectator session.
      *
      * cb - A SessionCallbacks structure which contains the callbacks you implement
      * to help .net synchronize the two games.  You must implement all functions in
      * cb, even if they do nothing but 'return true';
      *
      * game - The name of the game.  This is used internally for  for logging purposes only.
      *
      * num_players - The number of players which will be in this game.  The number of players
      * per session is fixed.  If you need to change the number of players or any player
      * disconnects, you must start a new session.
      *
      * input_size - The size of the game inputs which will be passsed to ggpo_add_local_input.
      *
      * local_port - The port  should bind to for UDP traffic.
      *
      * host_ip - The IP address of the host who will serve you the inputs for the game.  Any
      * player partcipating in the session can serve as a host.
      *
      * host_port - The port of the session on the host
      */
     ErrorCode
         ggpo_start_spectating
         (
             Session** session,
             const char* game,
             int num_players,
             int input_size,
             unsigned short local_port,
             char* host_ip,
             unsigned short host_port
         );

     /*
      * ggpo_close_session --
      * Used to close a session.  You must call ggpo_close_session to
      * free the resources allocated in ggpo_start_session.
      */
     ErrorCode
         ggpo_close_session(Session*);


     /*
      * ggpo_set_frame_delay --
      *
      * Change the amount of frames ggpo will delay local input.  Must be called
      * before the first call to ggpo_synchronize_input.
      */
     ErrorCode
         ggpo_set_frame_delay
         (
             Session*,
             PlayerHandle player,
             int frame_delay
         );

     /*
      * ggpo_idle --
      * Should be called periodically by your application to give .net
      * a chance to do some work.  Most packet transmissions and rollbacks occur
      * in ggpo_idle.
      *
      * timeout - The amount of time .net is allowed to spend in this function,
      * in milliseconds.
      */
     ErrorCode
         ggpo_idle
         (
             Session*,
             int timeout
         );

     /*
      * ggpo_add_local_input --
      *
      * Used to notify .net of inputs that should be trasmitted to remote
      * players.  ggpo_add_local_input must be called once every frame for
      * all player of type PLAYERTYPE_LOCAL.
      *
      * player - The player handle returned for this player when you called
      * ggpo_add_local_player.
      *
      * values - The controller inputs for this player.
      *
      * size - The size of the controller inputs.  This must be exactly equal to the
      * size passed into ggpo_start_session.
      */
     ErrorCode
         ggpo_add_local_input
         (
             Session*,
             PlayerHandle player,
             void* values,
             int size
         );

     /*
      * ggpo_synchronize_input --
      *
      * You should call ggpo_synchronize_input before every frame of execution,
      * including those frames which happen during rollback.
      *
      * values - When the function returns, the values parameter will contain
      * inputs for this frame for all players.  The values array must be at
      * least (size * players) large.
      *
      * size - The size of the values array.
      *
      * disconnect_flags - Indicated whether the input in slot (1 << flag) is
      * valid.  If a player has disconnected, the input in the values array for
      * that player will be zeroed and the i-th flag will be set.  For example,
      * if only player 3 has disconnected, disconnect flags will be 8 (i.e. 1 << 3).
      */
     ErrorCode
         ggpo_synchronize_input
         (
             Session*,
             void* values,
             int size,
             int* disconnect_flags
         );

     /*
      * ggpo_disconnect_player --
      *
      * Disconnects a remote player from a game.  Will return ERRORCODE_PLAYER_DISCONNECTED
      * if you try to disconnect a player who has already been disconnected.
      */
     ErrorCode
         ggpo_disconnect_player
         (
             Session*,
             PlayerHandle player
         );

     /*
      * ggpo_advance_frame --
      *
      * You should call ggpo_advance_frame to notify .net that you have
      * advanced your gamestate by a single frame.  You should call this everytime
      * you advance the gamestate by a frame, even during rollbacks.  .net
      * may call your save_state callback before this function returns.
      */
     ErrorCode  ggpo_advance_frame(Session*);

     /*
      * ggpo_get_network_stats --
      *
      * Used to fetch some statistics about the quality of the network connection.
      *
      * player - The player handle returned from the ggpo_add_player function you used
      * to add the remote player.
      *
      * stats - Out parameter to the network statistics.
      */
     ErrorCode
         ggpo_get_network_stats
         (
             Session*,
             PlayerHandle player,
             NetworkStats* stats
         );

     /*
      * ggpo_set_disconnect_timeout --
      *
      * Sets the disconnect timeout.  The session will automatically disconnect
      * from a remote peer if it has not received a packet in the timeout window.
      * You will be notified of the disconnect via a EVENTCODE_DISCONNECTED_FROM_PEER
      * event.
      *
      * Setting a timeout value of 0 will disable automatic disconnects.
      *
      * timeout - The time in milliseconds to wait before disconnecting a peer.
      */
     ErrorCode
         ggpo_set_disconnect_timeout
         (
             Session*,
             int timeout
         );

     /*
      * ggpo_set_disconnect_notify_start --
      *
      * The time to wait before the first EVENTCODE_NETWORK_INTERRUPTED timeout
      * will be sent.
      *
      * timeout - The amount of time which needs to elapse without receiving a packet
      *           before the EVENTCODE_NETWORK_INTERRUPTED event is sent.
      */
     ErrorCode
         ggpo_set_disconnect_notify_start
         (
             Session*,
             int timeout
         );
}


