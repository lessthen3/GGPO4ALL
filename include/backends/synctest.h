/* -----------------------------------------------------------------------
 * .net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#pragma once

#include "platform_common.h"
#include "backend.h"
#include "sync.h"
#include "ring_buffer.h"

namespace GGPO
{
     class SyncTestBackend : public IQuarkBackend
     {
     public:
         SyncTestBackend(char* gamename, int frames, int num_players);
         virtual ~SyncTestBackend();

         virtual ErrorCode DoPoll(int timeout);
         virtual ErrorCode AddPlayer(Player* player, PlayerHandle* handle);
         virtual ErrorCode AddLocalInput(PlayerHandle player, void* values, int size);
         virtual ErrorCode SyncInput(void* values, int size, int* disconnect_flags);
         virtual ErrorCode IncrementFrame(void);

     protected:
         struct SavedInfo
         {
             int frame;
             int checksum;
             string buf;
             GameInput input;
         };

         void LogSaveStates(SavedInfo& info);

     protected:
         Sync                   _sync;
         int                    _num_players;
         int                    _check_distance;
         int                    _last_verified;
         bool                   _rollingback;
         bool                   _running;
         char                   _game[128];

         GameInput                  _current_input;
         GameInput                  _last_input;
         RingBuffer<SavedInfo, 32>  _saved_frames;
     };
}

