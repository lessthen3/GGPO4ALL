/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#pragma once

#include "poll.h"
#include "udp_msg.h"
#include "ggponet.h"
#include "Buffers.h"

namespace GGPO
{
    #if defined(_WIN32) || defined(_WIN64)
    #include <winsock2.h>
    #include <ws2tcpip.h>

         typedef SOCKET GGPO_SOCKET;
    #define GGPO_INVALID_SOCKET (SOCKET)(~0)

    #define GGPO_GET_LAST_ERROR() WSAGetLastError()
    #define GGPO_CLOSE_SOCKET(__arg) closesocket(__arg)
    #define GGPO_SOCKET_ERROR_CODE WSAEWOULDBLOCK
    #else
        #include <sys/types.h>
        #include <sys/socket.h>
        #include <netinet/in.h>
        #include <arpa/inet.h>
        #include <fcntl.h>
        #include <unistd.h>
        #include <errno.h>


        typedef uint64_t GGPO_SOCKET;
        #define GGPO_INVALID_SOCKET (-1)

        #define GGPO_GET_LAST_ERROR() errno
        #define GGPO_CLOSE_SOCKET(__arg) close(__arg)
        #define GGPO_SOCKET_ERROR_CODE EWOULDBLOCK
    #endif

     constexpr auto GGPO_SOCKET_ERROR(-1);
     constexpr auto MAX_UDP_ENDPOINTS(16);

     static const int MAX_UDP_PACKET_SIZE = 4096;

     class Udp : public IPollSink
     {
     public:
         struct Stats
         {
             int      bytes_sent;
             int      packets_sent;
             float    kbps_sent;
         };

         struct Callbacks
         {
             virtual ~Callbacks() { }
             virtual void OnMsg(sockaddr_in& from, UdpMsg* msg, int len) = 0;
         };

     public:
         Udp();

         void Init(uint16_t port, Poll* p, Callbacks* callbacks);

         void SendTo(char* buffer, int len, int flags, struct sockaddr* dst, int destlen);

         virtual bool OnLoopPoll(void* cookie);

     public:
         ~Udp(void);

     protected:
         // Network transmission information
         GGPO_SOCKET _socket;

         // state management
         Callbacks* _callbacks;
         Poll* _poll;

     };
}
