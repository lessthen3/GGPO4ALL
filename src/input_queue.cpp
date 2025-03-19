/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "input_queue.h"

#define PREVIOUS_FRAME(offset)   (((offset) == 0) ? (INPUT_QUEUE_LENGTH - 1) : ((offset) - 1))

InputQueue::InputQueue(int input_size)
{
   Init(-1, input_size);
}

InputQueue::~InputQueue()
{
}

void
    InputQueue::Init(int id, int input_size)
    {
       _id = id;
       _head = 0;
       _tail = 0;
       _length = 0;
       _frame_delay = 0;
       _first_frame = true;
       _last_user_added_frame = GameInput::NullFrame;
       _first_incorrect_frame = GameInput::NullFrame;
       _last_frame_requested = GameInput::NullFrame;
       _last_added_frame = GameInput::NullFrame;

       _prediction.init(GameInput::NullFrame, NULL, input_size);

       /*
        * This is safe because we know the GameInput is a proper structure (as in,
        * no virtual methods, no contained classes, etc.).
        */
       memset(_inputs, 0, sizeof _inputs);

       for (int i = 0; i < ARRAY_SIZE(_inputs); i++)
       {
          _inputs[i].size = input_size;
       }
    }

int
    InputQueue::GetLastConfirmedFrame()
    {
        logger->LogAndPrint(format("returning last confirmed frame {}.", _last_added_frame), "input_queue.cpp", "info");
        return _last_added_frame;
    }

int
    InputQueue::GetFirstIncorrectFrame()
    const
    {
       return _first_incorrect_frame;
    }

void
    InputQueue::DiscardConfirmedFrames(int frame)
    {
       ASSERT(frame >= 0);

       if (_last_frame_requested != GameInput::NullFrame) 
       {
          frame = MIN(frame, _last_frame_requested);
       }

       logger->LogAndPrint(format("discarding confirmed frames up to {} (last_added:{} length:{} [head:{} tail:{}]).", frame, _last_added_frame, _length, _head, _tail), "input_queue.cpp", "info");

       if (frame >= _last_added_frame) 
       {
          _tail = _head;
       } 
       else 
       {
          int offset = frame - _inputs[_tail].frame + 1;
      
          logger->LogAndPrint(format("difference of {} frames.", offset), "input_queue.cpp", "info");
          ASSERT(offset >= 0);

          _tail = (_tail + offset) % INPUT_QUEUE_LENGTH;
          _length -= offset;
       }

       logger->LogAndPrint(format("after discarding, new tail is {} (frame:{}).", _tail, _inputs[_tail].frame), "input_queue.cpp", "info");
       ASSERT(_length >= 0);
    }

void
    InputQueue::ResetPrediction(int frame)
    {
       ASSERT(_first_incorrect_frame == GameInput::NullFrame || frame <= _first_incorrect_frame);

       logger->LogAndPrint(format("resetting all prediction errors back to frame {}.", frame), "input_queue.cpp", "info");

       /*
        * There's nothing really to do other than reset our prediction
        * state and the incorrect frame counter...
        */
       _prediction.frame = GameInput::NullFrame;
       _first_incorrect_frame = GameInput::NullFrame;
       _last_frame_requested = GameInput::NullFrame;
    }

bool
    InputQueue::GetConfirmedInput(int requested_frame, GameInput* input)
    {
       ASSERT(_first_incorrect_frame == GameInput::NullFrame || requested_frame < _first_incorrect_frame);
       int offset = requested_frame % INPUT_QUEUE_LENGTH; 

       if (_inputs[offset].frame != requested_frame)
       {
          return false;
       }

       *input = _inputs[offset];

       return true;
    }

bool
    InputQueue::GetInput(int requested_frame, GameInput* input)
    {
        logger->LogAndPrint(format("requesting input frame {}.", requested_frame), "input_queue.cpp", "info");

        /*
        * No one should ever try to grab any input when we have a prediction
        * error.  Doing so means that we're just going further down the wrong
        * path.  ASSERT this to verify that it's true.
        */
        ASSERT(_first_incorrect_frame == GameInput::NullFrame);

        /*
        * Remember the last requested frame number for later.  We'll need
        * this in AddInput() to drop out of prediction mode.
        */
        _last_frame_requested = requested_frame;

        ASSERT(requested_frame >= _inputs[_tail].frame);

        if (_prediction.frame == GameInput::NullFrame) 
        {
            /*
            * If the frame requested is in our range, fetch it out of the queue and
            * return it.
            */
            int offset = requested_frame - _inputs[_tail].frame;

            if (offset < _length) 
            {
                offset = (offset + _tail) % INPUT_QUEUE_LENGTH;
                ASSERT(_inputs[offset].frame == requested_frame);
                *input = _inputs[offset];
                logger->LogAndPrint(format("returning confirmed frame number {}.", input->frame), "input_queue.cpp", "info");
                return true;
            }

            /*
            * The requested frame isn't in the queue.  Bummer.  This means we need
            * to return a prediction frame.  Predict that the user will do the
            * same thing they did last time.
            */
            if (requested_frame == 0) 
            {
                logger->LogAndPrint("basing new prediction frame from nothing, you're client wants frame 0.", "input_queue.cpp", "info");
                _prediction.erase();
            } 
            else if (_last_added_frame == GameInput::NullFrame) 
            {
                logger->LogAndPrint("basing new prediction frame from nothing, since we have no frames yet.", "input_queue.cpp", "info");
                _prediction.erase();
            } 
            else 
            {
                logger->LogAndPrint(format("basing new prediction frame from previously added frame (queue entry:{}, frame:{}).", PREVIOUS_FRAME(_head), _inputs[PREVIOUS_FRAME(_head)].frame), "input_queue.cpp", "info");
                _prediction = _inputs[PREVIOUS_FRAME(_head)];
            }
            _prediction.frame++;
        }

        ASSERT(_prediction.frame >= 0);

        /*
        * If we've made it this far, we must be predicting.  Go ahead and
        * forward the prediction frame contents.  Be sure to return the
        * frame number requested by the client, though.
        */
        *input = _prediction;
        input->frame = requested_frame;
        logger->LogAndPrint(format("returning prediction frame number {} ({}).", input->frame, _prediction.frame), "input_queue.cpp", "info");

        return false;
    }

void
    InputQueue::AddInput(GameInput& input)
    {
        int new_frame;

        logger->LogAndPrint(format("adding input frame number {} to queue.", input.frame), "input_queue.cpp", "info");

        /*
        * These next two lines simply verify that inputs are passed in 
        * sequentially by the user, regardless of frame delay.
        */
        ASSERT(_last_user_added_frame == GameInput::NullFrame ||
                input.frame == _last_user_added_frame + 1);
        _last_user_added_frame = input.frame;

        /*
        * Move the queue head to the correct point in preparation to
        * input the frame into the queue.
        */
        new_frame = AdvanceQueueHead(input.frame);

        if (new_frame != GameInput::NullFrame) 
        {
            AddDelayedInputToQueue(input, new_frame);
        }
   
        /*
        * Update the frame number for the input.  This will also set the
        * frame to GameInput::NullFrame for frames that get dropped (by
        * design).
        */
        input.frame = new_frame;
    }

void
    InputQueue::AddDelayedInputToQueue(GameInput& input, int frame_number)
    {
        logger->LogAndPrint(format("adding delayed input frame number {} to queue.", frame_number), "input_queue.cpp", "info");

        ASSERT(input.size == _prediction.size);

        ASSERT(_last_added_frame == GameInput::NullFrame || frame_number == _last_added_frame + 1);

        ASSERT(frame_number == 0 || _inputs[PREVIOUS_FRAME(_head)].frame == frame_number - 1);

        /*
        * Add the frame to the back of the queue
        */ 
        _inputs[_head] = input;
        _inputs[_head].frame = frame_number;
        _head = (_head + 1) % INPUT_QUEUE_LENGTH;
        _length++;
        _first_frame = false;

        _last_added_frame = frame_number;

        if (_prediction.frame != GameInput::NullFrame) 
        {
            ASSERT(frame_number == _prediction.frame);

            /*
            * We've been predicting...  See if the inputs we've gotten match
            * what we've been predicting.  If so, don't worry about it.  If not,
            * remember the first input which was incorrect so we can report it
            * in GetFirstIncorrectFrame()
            */
            if (_first_incorrect_frame == GameInput::NullFrame && !_prediction.equal(input, true)) 
            {
                logger->LogAndPrint(format("frame {} does not match prediction.  marking error.", frame_number), "input_queue.cpp", "info");
                _first_incorrect_frame = frame_number;
            }

            /*
            * If this input is the same frame as the last one requested and we
            * still haven't found any mis-predicted inputs, we can dump out
            * of predition mode entirely!  Otherwise, advance the prediction frame
            * count up.
            */
            if (_prediction.frame == _last_frame_requested && _first_incorrect_frame == GameInput::NullFrame) 
            {
                logger->LogAndPrint("prediction is correct!  dumping out of prediction mode.", "input_queue.cpp", "info");
                _prediction.frame = GameInput::NullFrame;
            } 
            else 
            {
                _prediction.frame++;
            }
        }
        ASSERT(_length <= INPUT_QUEUE_LENGTH);
    }

int
    InputQueue::AdvanceQueueHead(int frame)
    {
        logger->LogAndPrint(format("advancing queue head to frame {}.", frame), "input_queue.cpp", "info");

        int expected_frame = _first_frame ? 0 : _inputs[PREVIOUS_FRAME(_head)].frame + 1;

        frame += _frame_delay;

        if (expected_frame > frame)
        {
            /*
            * This can occur when the frame delay has dropped since the last
            * time we shoved a frame into the system.  In this case, there's
            * no room on the queue.  Toss it.
            */
            logger->LogAndPrint(format("Dropping input frame {} (expected next frame to be {}).", frame, expected_frame), "input_queue.cpp", "info");
            return GameInput::NullFrame;
        }

        while (expected_frame < frame) 
        {
            /*
            * This can occur when the frame delay has been increased since the last
            * time we shoved a frame into the system.  We need to replicate the
            * last frame in the queue several times in order to fill the space
            * left.
            */
            logger->LogAndPrint(format("Adding padding frame {} to account for change in frame delay.", expected_frame), "input_queue.cpp", "info");
            GameInput &last_frame = _inputs[PREVIOUS_FRAME(_head)];     
            AddDelayedInputToQueue(last_frame, expected_frame);
            expected_frame++;
        }

        ASSERT(frame == 0 || frame == _inputs[PREVIOUS_FRAME(_head)].frame + 1);
        return frame;
    }
