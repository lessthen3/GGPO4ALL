/*******************************************************************
 *                                             GGPO4ALL v0.0.1
 *Created by Ranyodh Mandur - � 2024 and GroundStorm Studios, LLC. - � 2009
 *
 *                         Licensed under the MIT License (MIT).
 *                  For more details, see the LICENSE file or visit:
 *                        https://opensource.org/licenses/MIT
 *
 *              GGPO4ALL is an open-source rollback netcode library
********************************************************************/

#pragma once

 /*
  * Macros
  */

#ifndef ARRAY_SIZE
#  define ARRAY_SIZE(__arg)    (sizeof(__arg) / sizeof((__arg)[0]))
#endif

#ifndef MAX_INT
#  define MAX_INT          0xEFFFFFF
#endif

#ifndef MAX
#  define MAX(x, y)        (((x) > (y)) ? (x) : (y))
#endif

#ifndef MIN
#  define MIN(x, y)        (((x) < (y)) ? (x) : (y))
#endif

#define ASSERT(x)                                           
 //do {                                                     
 //   if (!(x)) {                                           
 //      //char assert_buf[1024];                             
 //      snprintf(assert_buf, sizeof(assert_buf) - 1, "Assertion: %s @ %s:%d (pid:%d)", #x, __FILE__, __LINE__, Platform::GetProcessID()); \
   //      //LogManager::GGPO_LOGGER().LogAndPrint(to_string(assert_buf), "", "");                           
   //      Platform::AssertFailed(assert_buf);                
   //      exit(0);                                           
   //   }                                                     
   //} while (false)

#include "LogManager.h"
#include <format>

extern LogManager* logger;

 // Platform-Specific Includes
#if defined(_WIN32) || defined(_WIN64)

	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <windows.h>
	#include <stdio.h>

	#include <cstdint>

	#include <timeapi.h> //apparently i need this idk where i took it out but i did tho lmfao

	class Platform
	{
	public:  // types
		typedef DWORD ProcessID;

	public:  // functions
		static ProcessID GetProcessID()
		{
			return GetCurrentProcessId();
		}

		static void AssertFailed(char* msg)
		{
			MessageBoxA(NULL, msg, "GGPO Assertion Failed", MB_OK | MB_ICONEXCLAMATION);
		}

		static uint32_t GetCurrentTimeMS()
		{
			return timeGetTime();
		}

		static int GetConfigInt(const char* name);
		static bool GetConfigBool(const char* name);
	};

#elif defined(__linux__) or defined(__APPLE__)

	#include <time.h>
	#include <cstdint>

	#include <stdio.h>
	#include <stdarg.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <stdlib.h>

	class Platform 
	{
	public:  // types
		typedef pid_t ProcessID;

	public:  // functions
		static ProcessID GetProcessID()
		{
			return getpid();
		}

		static void AssertFailed(const char* msg) { } //idek ill figure out whether i like the MessageBoxA thing from windows i feel like regular assert is fine but whatever
		static uint32_t GetCurrentTimeMS();
	};

#else

	#error Unsupported platform!

#endif

