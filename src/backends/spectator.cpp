/* -----------------------------------------------------------------------
 * .net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "../include/backends/spectator.h"

namespace GGPO
{

     SpectatorBackend::SpectatorBackend
     (
         const char* gamename,
         uint16_t localport,
         int num_players,
         int input_size,
         char* hostip,
         uint16_t hostport
     ) :
         _num_players(num_players),
         _input_size(input_size),
         _next_input_to_send(0)
     {
         _synchronizing = true;

         for (int i = 0; i < ARRAY_SIZE(_inputs); i++)
         {
             _inputs[i].frame = -1;
         }

         /*
          * Initialize the UDP port
          */
         _udp.Init(localport, &_poll, this);

         /*
          * Init the host endpoint
          */
         _host.Init(&_udp, _poll, 0, hostip, hostport, NULL);
         _host.Synchronize();

         /*
          * Preload the ROM
          */
          //_callbacks.begin_game(gamename); //?????????????????????????????????
     }

     SpectatorBackend::~SpectatorBackend()
     {
     }

     ErrorCode
         SpectatorBackend::DoPoll(int timeout)
     {
         _poll.Pump(0);

         PollUdpProtocolEvents();
         return ErrorCode::OK;
     }

     ErrorCode
         SpectatorBackend::SyncInput
         (
             void* values,
             int size,
             int* disconnect_flags
         )
     {
         // Wait until we've started to return inputs.
         if (_synchronizing)
         {
             return ErrorCode::NOT_SYNCHRONIZED;
         }

         GameInput& input = _inputs[_next_input_to_send % SPECTATOR_FRAME_BUFFER_SIZE];

         if (input.frame < _next_input_to_send)
         {
             // Haven't received the input from the host yet.  Wait
             return ErrorCode::PREDICTION_THRESHOLD;
         }
         if (input.frame > _next_input_to_send)
         {
             // The host is way way way far ahead of the spectator.  How'd this
             // happen?  Anyway, the input we need is gone forever.
             return ErrorCode::GENERAL_FAILURE;
         }

         ASSERT(size >= _input_size * _num_players);
         memcpy(values, input.bits, _input_size * _num_players);

         if (disconnect_flags)
         {
             *disconnect_flags = 0; // xxx: should get them from the host!
         }

         _next_input_to_send++;

         return ErrorCode::OK;
     }

     ErrorCode
         SpectatorBackend::IncrementFrame(void)
     {
         logger->LogAndPrint(format("End of frame ({})...", _next_input_to_send - 1), "spectator.cpp", "info");
         DoPoll(0);
         PollUdpProtocolEvents();

         return ErrorCode::OK;
     }

     void
         SpectatorBackend::PollUdpProtocolEvents(void)
     {
         UdpProtocol::Event evt;

         while (_host.GetEvent(evt))
         {
             OnUdpProtocolEvent(evt);
         }
     }

     void
         SpectatorBackend::OnUdpProtocolEvent(UdpProtocol::Event& evt)
     {
         Event info;

         switch (evt.type)
         {
         case UdpProtocol::Event::Connected:
             info.code = EventCode::ConnectedToPeer;
             info.u.connected.player = 0;
             _callbacks.on_event(&info);
             break;

         case UdpProtocol::Event::Synchronizing:
             info.code = EventCode::SynchronizingWithPeer;
             info.u.synchronizing.player = 0;
             info.u.synchronizing.count = evt.u.synchronizing.count;
             info.u.synchronizing.total = evt.u.synchronizing.total;
             _callbacks.on_event(&info);
             break;

         case UdpProtocol::Event::Synchronzied:
             if (_synchronizing)
             {
                 info.code = EventCode::SynchronizedWithPeer;
                 info.u.synchronized.player = 0;
                 _callbacks.on_event(&info);

                 info.code = EventCode::Running;
                 _callbacks.on_event(&info);
                 _synchronizing = false;
             }
             break;

         case UdpProtocol::Event::NetworkInterrupted:
             info.code = EventCode::ConnectionInterrupted;
             info.u.connection_interrupted.player = 0;
             info.u.connection_interrupted.disconnect_timeout = evt.u.network_interrupted.disconnect_timeout;
             _callbacks.on_event(&info);
             break;

         case UdpProtocol::Event::NetworkResumed:
             info.code = EventCode::ConnectionResumed;
             info.u.connection_resumed.player = 0;
             _callbacks.on_event(&info);
             break;

         case UdpProtocol::Event::Disconnected:
             info.code = EventCode::DisconnectedFromPeer;
             info.u.disconnected.player = 0;
             _callbacks.on_event(&info);
             break;

         case UdpProtocol::Event::Input:
             GameInput& input = evt.u.input.input;

             _host.SetLocalFrameNumber(input.frame);
             _host.SendInputAck();
             _inputs[input.frame % SPECTATOR_FRAME_BUFFER_SIZE] = input;
             break;
         }
     }

     void
         SpectatorBackend::OnMsg
         (
             sockaddr_in& from,
             UdpMsg* msg,
             int len
         )
     {
         if (_host.HandlesMsg(from, msg))
         {
             _host.OnMsg(msg, len);
         }
     }
}

