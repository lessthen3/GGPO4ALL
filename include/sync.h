/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#pragma once

#include "platform_common.h"
#include "ggponet.h"
#include "game_input.h"
#include "input_queue.h"
#include "ring_buffer.h"
#include "network/udp_msg.h"

namespace GGPO
{
     class SyncTestBackend;

     class Sync
     {
     public:
         struct Config
         {
             int                     num_prediction_frames;
             int                     num_players;
             int                     input_size;
         };
         struct Event
         {
             enum
             {
                 ConfirmedInput,
             } type;
             union
             {
                 struct confirmedInput
                 {
                     GameInput   input;
                 };
             } u;
         };

     public:
         Sync(UdpMsg::connect_status* connect_status);
         virtual ~Sync();

         void Init(Config& config);

         void SetLastConfirmedFrame(int frame);
         void SetFrameDelay(int queue, int delay);
         bool AddLocalInput(int queue, GameInput& input);
         void AddRemoteInput(int queue, GameInput& input);
         int GetConfirmedInputs(void* values, int size, int frame);
         int SynchronizeInputs(void* values, int size);

         void CheckSimulation(int timeout);
         void AdjustSimulation(int seek_to);
         void IncrementFrame(void);

         int
             GetFrameCount()
             const
         {
             return _framecount;
         }

         bool
             InRollback()
             const
         {
             return _rollingback;
         }

         bool GetEvent(Event& e);

     protected:
         friend SyncTestBackend;

         struct SavedFrame
         {
             string buf;
             int      frame;
             int      checksum;
             SavedFrame() : buf(), frame(-1), checksum(0) { }
         };
         struct SavedState
         {
             SavedFrame frames[MAX_PREDICTION_FRAMES + 2];
             int head;
         };

         void LoadFrame(int frame);
         void SaveCurrentFrame();
         int FindSavedFrameIndex(int frame);
         SavedFrame& GetLastSavedFrame();

         bool CreateQueues(Config& config);
         bool CheckSimulationConsistency(int* seekTo);
         void ResetPrediction(int frameNumber);

     protected:
         SavedState     _savedstate;
         Config         _config;

         bool           _rollingback;
         int            _last_confirmed_frame;
         int            _framecount;
         int            _max_prediction_frames;

         InputQueue* _input_queues;

         RingBuffer<Event, 32> _event_queue;
         UdpMsg::connect_status* _local_connect_status;
     };

}
