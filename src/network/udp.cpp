/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "../include/network/udp.h"

namespace GGPO
{
     static GGPO_SOCKET
         CreateSocket(uint16_t bind_port, int retries)
     {
         GGPO_SOCKET f_Socket;
         sockaddr_in sin;
         uint16_t port;
         int optval = 1;

         f_Socket = socket(AF_INET, SOCK_DGRAM, 0);
         setsockopt(f_Socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof optval);
         setsockopt(f_Socket, SOL_SOCKET, SO_DONTLINGER, (const char*)&optval, sizeof optval);

         // non-blocking...
        #if defined(_WIN32)
             u_long iMode = 1;
             ioctlsocket(f_Socket, FIONBIO, &iMode);
        #else
             int flags = fcntl(s, F_GETFL, 0);
             fcntl(s, F_SETFL, flags | O_NONBLOCK);
        #endif

         sin.sin_family = AF_INET;
         sin.sin_addr.s_addr = htonl(INADDR_ANY);

         for (port = bind_port; port <= bind_port + retries; port++)
         {
             sin.sin_port = htons(port);

             if (bind(f_Socket, (sockaddr*)&sin, sizeof sin) != GGPO_SOCKET_ERROR)
             {
                 logger->LogAndPrint(format("Udp bound to port: {}.", port), "udp.cpp", LogManager::LogLevel::Info);
                 return f_Socket;
             }
         }

         GGPO_CLOSE_SOCKET(f_Socket);

         return GGPO_INVALID_SOCKET;
     }

     Udp::Udp() :
         _socket(GGPO_INVALID_SOCKET),
         _callbacks(NULL)
     {
     }

     Udp::~Udp(void)
     {
         if (_socket != GGPO_INVALID_SOCKET)
         {
             GGPO_CLOSE_SOCKET(_socket);
             _socket = GGPO_INVALID_SOCKET;
         }
     }

     void
         Udp::Init(uint16_t port, Poll* poll, Callbacks* callbacks)
     {
         _callbacks = callbacks;

         _poll = poll;
         _poll->RegisterLoop(this);

         logger->LogAndPrint(format("binding udp socket to port {}.", port), "udp.cpp", LogManager::LogLevel::Info);
         _socket = CreateSocket(port, 0);
     }

     void
         Udp::SendTo(char* buffer, int len, int flags, struct sockaddr* dst, int destlen)
     {
         struct sockaddr_in* to = (struct sockaddr_in*)dst;

         int res = sendto(_socket, buffer, len, flags, dst, destlen);

         if (res == GGPO_SOCKET_ERROR)
         {
             DWORD err = GGPO_GET_LAST_ERROR();
             logger->LogAndPrint(format("unknown error in sendto (erro: {}  wsaerr: {}).", res, err), "udp.cpp", LogManager::LogLevel::Error);
             ASSERT(FALSE && "Unknown error in sendto");
         }

         char dst_ip[1024];

         logger->LogAndPrint(format("sent packet length {} to {}:{} (ret:{}).", len, inet_ntop(AF_INET, (void*)&to->sin_addr, dst_ip, ARRAY_SIZE(dst_ip)), ntohs(to->sin_port), res), "udp.cpp", LogManager::LogLevel::Error);
     }

     bool
         Udp::OnLoopPoll(void* cookie)
     {
         uint8_t recv_buf[MAX_UDP_PACKET_SIZE];
         sockaddr_in recv_addr;
         int recv_addr_len;

         for (;;)
         {
             recv_addr_len = sizeof(recv_addr);
             int len = recvfrom(_socket, (char*)recv_buf, MAX_UDP_PACKET_SIZE, 0, (struct sockaddr*)&recv_addr, &recv_addr_len);

             // TODO: handle len == 0... indicates a disconnect.

             if (len == -1)
             {
                 int error = GGPO_GET_LAST_ERROR();

                 if (error != GGPO_SOCKET_ERROR_CODE)
                 {
                     logger->LogAndPrint(format("recvfrom GGPO_GET_LAST_ERROR returned {} ({}).", error, error), "udp.cpp", LogManager::LogLevel::Error);
                 }

                 break;
             }
             else if (len > 0)
             {
                 char src_ip[1024];
                 logger->LogAndPrint(format("recvfrom returned (len:{}  from:{}:{}).", len, inet_ntop(AF_INET, (void*)&recv_addr.sin_addr, src_ip, ARRAY_SIZE(src_ip)), ntohs(recv_addr.sin_port)), "udp.cpp", LogManager::LogLevel::Error);
                 UdpMsg* msg = (UdpMsg*)recv_buf;
                 _callbacks->OnMsg(recv_addr, msg, len);
             }
         }
         return true;
     }
}
