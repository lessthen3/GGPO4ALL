/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#pragma once

#if defined(__linux__) or defined(__APPLE__)

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

#include "macros.h"

class Platform {
public:  // types
   typedef pid_t ProcessID;

public:  // functions
   static ProcessID GetProcessID() 
   { 
	   return getpid(); 
   }

   static void AssertFailed(const char* msg) { }
   static uint32_t GetCurrentTimeMS();
};

#endif

