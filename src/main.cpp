/* -----------------------------------------------------------------------
 * .net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "backends/p2p.h"
#include "backends/synctest.h"
#include "backends/spectator.h"
#include "ggponet.h"

namespace GGPO
{
     ErrorCode
         ggpo_start_session
         (
             Session** session,
             const char* game,
             int num_players,
             int input_size,
             unsigned short localport
         )
     {
         *session = (Session*)new Peer2PeerBackend
         (
             game,
             localport,
             num_players,
             input_size
         );

         return ErrorCode::OK;
     }

     ErrorCode
         ggpo_add_player
         (
             Session* ggpo,
             Player* player,
             PlayerHandle* handle
         )
     {
         if (not ggpo)
         {
             return ErrorCode::INVALID_SESSION;
         }

         return ggpo->AddPlayer(player, handle);
     }



     ErrorCode
         ggpo_start_synctest
         (
             Session** ggpo,
             char* game,
             int num_players,
             int input_size,
             int frames
         )
     {
         *ggpo = (Session*)new SyncTestBackend(game, frames, num_players);
         return ErrorCode::OK;
     }

     ErrorCode
         ggpo_set_frame_delay
         (
             Session* ggpo,
             PlayerHandle player,
             int frame_delay
         )
     {
         if (not ggpo)
         {
             return ErrorCode::INVALID_SESSION;
         }

         return ggpo->SetFrameDelay(player, frame_delay);
     }

     ErrorCode
         ggpo_idle
         (
             Session* ggpo,
             int timeout
         )
     {
         if (not ggpo)
         {
             return ErrorCode::INVALID_SESSION;
         }
         return ggpo->DoPoll(timeout);
     }

     ErrorCode
         ggpo_add_local_input
         (
             Session* ggpo,
             PlayerHandle player,
             void* values,
             int size
         )
     {
         if (not ggpo)
         {
             return ErrorCode::INVALID_SESSION;
         }
         return ggpo->AddLocalInput(player, values, size);
     }

     ErrorCode
         ggpo_synchronize_input
         (
             Session* ggpo,
             void* values,
             int size,
             int* disconnect_flags
         )
     {
         if (not ggpo)
         {
             return ErrorCode::INVALID_SESSION;
         }
         return ggpo->SyncInput(values, size, disconnect_flags);
     }

     ErrorCode
         ggpo_disconnect_player
         (
             Session* ggpo,
             PlayerHandle player
         )
     {
         if (not ggpo)
         {
             return ErrorCode::INVALID_SESSION;
         }
         return ggpo->DisconnectPlayer(player);
     }

     ErrorCode
         ggpo_advance_frame(Session* ggpo)
     {
         if (not ggpo)
         {
             return ErrorCode::INVALID_SESSION;
         }
         return ggpo->IncrementFrame();
     }

     ErrorCode
         ggpo_client_chat(Session* ggpo, char* text)
     {
         if (not ggpo)
         {
             return ErrorCode::INVALID_SESSION;
         }
         return ggpo->Chat(text);
     }

     ErrorCode
         ggpo_get_network_stats
         (
             Session* ggpo,
             PlayerHandle player,
             NetworkStats* stats
         )
     {
         if (not ggpo)
         {
             return ErrorCode::INVALID_SESSION;
         }
         return ggpo->GetNetworkStats(stats, player);
     }


     ErrorCode
         ggpo_close_session(Session* ggpo)
     {
         if (not ggpo)
         {
             return ErrorCode::INVALID_SESSION;
         }
         delete ggpo;
         return ErrorCode::OK;
     }

     ErrorCode
         ggpo_set_disconnect_timeout(Session* ggpo, int timeout)
     {
         if (not ggpo)
         {
             return ErrorCode::INVALID_SESSION;
         }
         return ggpo->SetDisconnectTimeout(timeout);
     }

     ErrorCode
         ggpo_set_disconnect_notify_start(Session* ggpo, int timeout)
     {
         if (not ggpo)
         {
             return ErrorCode::INVALID_SESSION;
         }
         return ggpo->SetDisconnectNotifyStart(timeout);
     }

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
         )
     {
         *session = (Session*) new SpectatorBackend
         (
             game,
             local_port,
             num_players,
             input_size,
             host_ip,
             host_port
         );
         return ErrorCode::OK;
     }
}

