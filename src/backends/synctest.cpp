/* -----------------------------------------------------------------------
 * .net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "../include/backends/synctest.h"

namespace GGPO
{
     SyncTestBackend::SyncTestBackend
     (
         char* gamename,
         int frames,
         int num_players
     ) :
         _sync(NULL)
     {
         _num_players = num_players;
         _check_distance = frames;
         _last_verified = 0;
         _rollingback = false;
         _running = false;
         _current_input.erase();
         strcpy_s(_game, gamename);

         /*
          * Initialize the synchronziation layer
          */
         Sync::Config config = { 0 };
         config.num_prediction_frames = MAX_PREDICTION_FRAMES;
         _sync.Init(config);

         /*
          * Preload the ROM
          */
          //_callbacks.begin_game(gamename); ?????????????????????????????
     }

     SyncTestBackend::~SyncTestBackend()
     {
     }

     ErrorCode
         SyncTestBackend::DoPoll(int timeout)
     {
         if (not _running)
         {
             Event info;

             info.code = EventCode::Running;
             _callbacks.on_event(&info);
             _running = true;
         }
         return ErrorCode::OK;
     }

     ErrorCode
         SyncTestBackend::AddPlayer(Player* player, PlayerHandle* handle)
     {
         if (player->player_num < 1 || player->player_num > _num_players)
         {
             return ErrorCode::PLAYER_OUT_OF_RANGE;
         }

         *handle = (PlayerHandle)(player->player_num - 1);
         return ErrorCode::OK;
     }

     ErrorCode
         SyncTestBackend::AddLocalInput(PlayerHandle player, void* values, int size)
     {
         if (not _running)
         {
             return ErrorCode::NOT_SYNCHRONIZED;
         }

         int index = (int)player;

         for (int i = 0; i < size; i++)
         {
             _current_input.bits[(index * size) + i] |= ((char*)values)[i];
         }

         return ErrorCode::OK;
     }

     ErrorCode
         SyncTestBackend::SyncInput
         (
             void* values,
             int size,
             int* disconnect_flags
         )
     {
         if (_rollingback) //HERES THE THING
         {
             _last_input = _saved_frames.front().input;
         }
         else
         {
             if (_sync.GetFrameCount() == 0)
             {
                 _sync.SaveCurrentFrame();
             }
             _last_input = _current_input;
         }

         memcpy(values, _last_input.bits, size);

         if (disconnect_flags)
         {
             *disconnect_flags = 0;
         }

         return ErrorCode::OK;
     }

     ErrorCode
         SyncTestBackend::IncrementFrame(void)
     {
         _sync.IncrementFrame();
         _current_input.erase();

         logger->LogAndPrint(format("End of frame({})...", _sync.GetFrameCount()), "synctest.cpp", "info");

         if (_rollingback)
         {
             return ErrorCode::OK;
         }

         int frame = _sync.GetFrameCount();
         // Hold onto the current frame in our queue of saved states.  We'll need
         // the checksum later to verify that our replay of the same frame got the
         // same results.
         SavedInfo info;

         info.frame = frame;
         info.input = _last_input;
         info.buf = (_sync.GetLastSavedFrame().buf);
         info.checksum = _sync.GetLastSavedFrame().checksum;

         _saved_frames.push(info);

         if (frame - _last_verified == _check_distance)
         {
             // We've gone far enough ahead and should now start replaying frames.
             // Load the last verified frame and set the rollback flag to true.
             _sync.LoadFrame(_last_verified);

             _rollingback = true;

             while (not _saved_frames.empty())
             {
                 _callbacks.advance_frame(0);

                 // Verify that the checksumn of this frame is the same as the one in our
                 // list.
                 info = _saved_frames.front();
                 _saved_frames.pop();

                 if (info.frame != _sync.GetFrameCount())
                 {
                     logger->LogAndPrint(format("Frame number {} does not match saved frame number {}", info.frame, frame), "synctest.cpp", "error");
                     logger->LogAndPrint(format("Program will now exit with error: {}", ErrorToString(ErrorCode::FATAL_DESYNC)), "synctest.cpp", "error");
                     exit(static_cast<int>(ErrorCode::FATAL_DESYNC)); //RAISESYNC ERRROR WAS HERE
                 }

                 int checksum = _sync.GetLastSavedFrame().checksum;

                 if (info.checksum != checksum)
                 {
                     logger->LogAndPrint(format("Checksum for frame {} does not match saved ({} != {})", frame, checksum, info.checksum), "synctest.cpp", "error");
                     logger->LogAndPrint(format("Program will now exit with error: {}", ErrorToString(ErrorCode::FATAL_DESYNC)), "synctest.cpp", "error");
                     exit(static_cast<int>(ErrorCode::FATAL_DESYNC)); //RAISESYNC ERRROR WAS HERE
                 }

                 logger->LogAndPrint(format("Checksum {} for frame {} matches.", checksum, info.frame), "synctest.cpp", "info");
             }

             _last_verified = frame;
             _rollingback = false;
         }

         return ErrorCode::OK;
     }

     void
         SyncTestBackend::LogSaveStates(SavedInfo& info)
     {
         logger->LogAndPrint(format("state-{}-original and {}", _sync.GetFrameCount(), info.buf), "synctest.cpp", "info");

         logger->LogAndPrint(format("state-{}-replay and {}", _sync.GetFrameCount(), _sync.GetLastSavedFrame().buf), "synctest.cpp", "info");
     }
}
