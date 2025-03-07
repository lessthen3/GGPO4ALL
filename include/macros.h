#pragma once

/*
 * Macros
 */
#define ASSERT(x)                                           \
   do {                                                     \
      if (!(x)) {                                           \
         char assert_buf[1024];                             \
         snprintf(assert_buf, sizeof(assert_buf) - 1, "Assertion: %s @ %s:%d (pid:%d)", #x, __FILE__, __LINE__, Platform::GetProcessID()); \
         Log("%s\n", assert_buf);                           \
         Log("\n");                                         \
         Log("\n");                                         \
         Log("\n");                                         \
         Platform::AssertFailed(assert_buf);                \
         exit(0);                                           \
      }                                                     \
   } while (false)

#ifndef ARRAY_SIZE
#  define ARRAY_SIZE(a)    (sizeof(a) / sizeof((a)[0]))
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