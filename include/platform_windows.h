/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#pragma once

#if defined(_WIN32) or defined(_WIN64)

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>

#include <cstdint>

#include "macros.h"

class Platform {
public:  // types
   typedef DWORD ProcessID;

public:  // functions
   static ProcessID GetProcessID() 
   { 
	   return GetCurrentProcessId(); 
   }

   static void AssertFailed(char *msg) 
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

#endif