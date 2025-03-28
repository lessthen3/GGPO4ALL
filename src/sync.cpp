/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "sync.h"
 
namespace GGPO 
{

     Sync::Sync(UdpMsg::connect_status* connect_status) :
         _local_connect_status(connect_status),
         _input_queues(NULL)
     {
         _framecount = 0;
         _last_confirmed_frame = -1;
         _max_prediction_frames = 0;
         memset(&_savedstate, 0, sizeof(_savedstate));
     }

     Sync::~Sync()
     {
         /*
          * Delete frames manually here rather than in a destructor of the SavedFrame
          * structure so we can efficently copy frames via weak references.
          */
         for (int i = 0; i < ARRAY_SIZE(_savedstate.frames); i++)
         {
             _callbacks.free_buffer((void*)&_savedstate.frames[i].buf); //lmfao this is ridiculous
         }
         delete[] _input_queues;
         _input_queues = NULL;
     }

     void
         Sync::Init(Sync::Config& config)
     {
         _config = config;
         _framecount = 0;
         _rollingback = false;

         _max_prediction_frames = config.num_prediction_frames;

         CreateQueues(config);
     }

     void
         Sync::SetLastConfirmedFrame(int frame)
     {
         _last_confirmed_frame = frame;
         if (_last_confirmed_frame > 0) 
         {
             for (int i = 0; i < _config.num_players; i++) 
             {
                 _input_queues[i].DiscardConfirmedFrames(frame - 1);
             }
         }
     }

     bool
         Sync::AddLocalInput(int queue, GameInput& input)
     {
         int frames_behind = _framecount - _last_confirmed_frame;

         if (_framecount >= _max_prediction_frames && frames_behind >= _max_prediction_frames)
         {
             logger->LogAndPrint("Rejecting input from emulator: reached prediction barrier.", "sync.cpp", LogManager::LogLevel::Info);
             return false;
         }

         if (_framecount == 0)
         {
             SaveCurrentFrame();
         }

         logger->LogAndPrint(format("Sending undelayed local frame {} to queue {}.", _framecount, queue), "sync.cpp", LogManager::LogLevel::Info);

         input.frame = _framecount;
         _input_queues[queue].AddInput(input);

         return true;
     }

     void
         Sync::AddRemoteInput(int queue, GameInput& input)
     {
         _input_queues[queue].AddInput(input);
     }

     int
         Sync::GetConfirmedInputs(void* values, int size, int frame)
     {
         int disconnect_flags = 0;
         char* output = (char*)values;

         ASSERT(size >= _config.num_players * _config.input_size);

         memset(output, 0, size);

         for (int i = 0; i < _config.num_players; i++)
         {
             GameInput input;
             if (_local_connect_status[i].disconnected && frame > _local_connect_status[i].last_frame)
             {
                 disconnect_flags |= (1 << i);
                 input.erase();
             }
             else
             {
                 _input_queues[i].GetConfirmedInput(frame, &input);
             }
             memcpy(output + (i * _config.input_size), input.bits, _config.input_size);
         }
         return disconnect_flags;
     }

     int
         Sync::SynchronizeInputs(void* values, int size)
     {
         int disconnect_flags = 0;
         char* output = (char*)values;

         ASSERT(size >= _config.num_players * _config.input_size);

         memset(output, 0, size);
         for (int i = 0; i < _config.num_players; i++)
         {
             GameInput input;

             if (_local_connect_status[i].disconnected and _framecount > _local_connect_status[i].last_frame)
             {
                 disconnect_flags |= (1 << i);
                 input.erase();
             }
             else
             {
                 _input_queues[i].GetInput(_framecount, &input);
             }
             memcpy(output + (i * _config.input_size), input.bits, _config.input_size);
         }
         return disconnect_flags;
     }

     void
         Sync::CheckSimulation(int timeout)
     {
         int seek_to;
         if (not CheckSimulationConsistency(&seek_to))
         {
             AdjustSimulation(seek_to);
         }
     }

     void
         Sync::IncrementFrame(void)
     {
         _framecount++;
         SaveCurrentFrame();
     }

     void
         Sync::AdjustSimulation(int seek_to)
     {
         int framecount = _framecount;
         int count = _framecount - seek_to;

         logger->LogAndPrint("Catching up", "sync.cpp", LogManager::LogLevel::Info);
         _rollingback = true;

         /*
          * Flush our input queue and load the last frame.
          */
         LoadFrame(seek_to);
         ASSERT(_framecount == seek_to);

         /*
          * Advance frame by frame (stuffing notifications back to
          * the master).
          */
         ResetPrediction(_framecount);

         for (int i = 0; i < count; i++)
         {
             _callbacks.advance_frame(0);
         }

         ASSERT(_framecount == framecount);

         _rollingback = false;

         logger->LogAndPrint("---", "sync.cpp", LogManager::LogLevel::Info); //?????????????????????????
     }

     void
         Sync::LoadFrame(int frame)
     {
         // find the frame in question
         if (frame == _framecount)
         {
             logger->LogAndPrint("Skipping NOP.", "sync.cpp", LogManager::LogLevel::Info);
             return;
         }

         // Move the head pointer back and load it up
         _savedstate.head = FindSavedFrameIndex(frame);
         SavedFrame* state = _savedstate.frames + _savedstate.head;

         logger->LogAndPrint(format("=== Loading frame info {} (checksum: {}).", state->frame, state->checksum), "sync.cpp", LogManager::LogLevel::Info);

         ASSERT(state->buf and state->cbuf);

         _callbacks.load_game_state(state->buf);

         // Reset framecount and the head of the state ring-buffer to point in
         // advance of the current frame (as if we had just finished executing it).
         _framecount = state->frame;
         _savedstate.head = (_savedstate.head + 1) % ARRAY_SIZE(_savedstate.frames);
     }

     void
         Sync::SaveCurrentFrame()
     {
         /*
          * See StateCompress for the real save feature implemented by FinalBurn.
          * Write everything into the head, then advance the head pointer.
          */
         SavedFrame* state = _savedstate.frames + _savedstate.head;

         state->frame = _framecount;
         _callbacks.save_game_state(state->buf, &state->frame, &state->checksum, state->frame);

         logger->LogAndPrint(format("=== Saved frame info {} (checksum: {}).", state->frame, state->checksum), "sync.cpp", LogManager::LogLevel::Info);
         _savedstate.head = (_savedstate.head + 1) % ARRAY_SIZE(_savedstate.frames);
     }

     Sync::SavedFrame&
         Sync::GetLastSavedFrame()
     {
         int i = _savedstate.head - 1;

         if (i < 0)
         {
             i = ARRAY_SIZE(_savedstate.frames) - 1;
         }

         return _savedstate.frames[i];
     }


     int
         Sync::FindSavedFrameIndex(int frame)
     {
         int i, count = ARRAY_SIZE(_savedstate.frames);

         for (i = 0; i < count; i++)
         {
             if (_savedstate.frames[i].frame == frame)
             {
                 break;
             }
         }
         if (i == count)
         {
             ASSERT(FALSE);
         }

         return i;
     }


     bool
         Sync::CreateQueues(Config& config)
     {
         delete[] _input_queues;
         _input_queues = new InputQueue[_config.num_players];

         for (int i = 0; i < _config.num_players; i++)
         {
             _input_queues[i].Init(i, _config.input_size);
         }

         return true;
     }

     bool
         Sync::CheckSimulationConsistency(int* seekTo)
     {
         int first_incorrect = GameInput::NullFrame;

         for (int i = 0; i < _config.num_players; i++)
         {
             int incorrect = _input_queues[i].GetFirstIncorrectFrame();
             logger->LogAndPrint(format("considering incorrect frame {} reported by queue {}.", incorrect, i), "sync.cpp", LogManager::LogLevel::Info);

             if (incorrect != GameInput::NullFrame and (first_incorrect == GameInput::NullFrame or incorrect < first_incorrect))
             {
                 first_incorrect = incorrect;
             }
         }

         if (first_incorrect == GameInput::NullFrame)
         {
             logger->LogAndPrint("prediction ok.  proceeding.", "sync.cpp", LogManager::LogLevel::Info);
             return true;
         }

         *seekTo = first_incorrect;

         return false;
     }

     void
         Sync::SetFrameDelay(int queue, int delay)
     {
         _input_queues[queue].SetFrameDelay(delay);
     }


     void
         Sync::ResetPrediction(int frameNumber)
     {
         for (int i = 0; i < _config.num_players; i++)
         {
             _input_queues[i].ResetPrediction(frameNumber);
         }
     }


     bool
         Sync::GetEvent(Event& e)
     {
         if (_event_queue.CurrentSize())
         {
             e = _event_queue.Front();
             _event_queue.Pop();
             return true;
         }

         return false;
     }
}


