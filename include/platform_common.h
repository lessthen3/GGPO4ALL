/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#pragma once
/*
 * Keep the compiler happy
 */

/*
 * Disable specific compiler warnings
 *   4018 - '<' : signed/unsigned mismatch
 *   4100 - 'xxx' : unreferenced formal parameter
 *   4127 - conditional expression is constant
 *   4201 - nonstandard extension used : nameless struct/union
 *   4389 - '!=' : signed/unsigned mismatch
 *   4800 - 'int' : forcing value to bool 'true' or 'false' (performance warning)
 */
#pragma warning(disable: 4018 4100 4127 4201 4389 4800)

#include "LogManager.h"
#include <format>


extern LogManager* logger;

 // Platform-Specific Includes
#if defined(_WIN32) || defined(_WIN64)
	#include "platform_windows.h"
#elif defined(__linux__) || defined(__APPLE__)
	#include "platform_linux.h"
#else
	#error Unsupported platform!
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

