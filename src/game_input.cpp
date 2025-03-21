/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "game_input.h"

namespace GGPO
 {
     void
         GameInput::init
         (
             int iframe,
             char* ibits,
             int isize,
             int offset
         )
     {
         ASSERT(isize);
         ASSERT(isize <= GAMEINPUT_MAX_BYTES);
         frame = iframe;
         size = isize;
         memset(bits, 0, sizeof(bits));

         if (ibits)
         {
             memcpy(bits + (offset * isize), ibits, isize);
         }
     }

     void
         GameInput::init(int iframe, char* ibits, int isize)
     {
         ASSERT(isize);
         ASSERT(isize <= GAMEINPUT_MAX_BYTES * GAMEINPUT_MAX_PLAYERS);

         frame = iframe;
         size = isize;
         memset(bits, 0, sizeof(bits));

         if (ibits)
         {
             memcpy(bits, ibits, isize);
         }
     }

     void
         GameInput::desc
         (
             char* buf,
             size_t buf_size,
             bool show_frame
         )
         const
     {
         ASSERT(size);
         size_t remaining = buf_size;

         if (show_frame)
         {
             remaining -= sprintf_s(buf, buf_size, "(frame:%d size:%d ", frame, size);
         }
         else
         {
             remaining -= sprintf_s(buf, buf_size, "(size:%d ", size);
         }

         for (int i = 0; i < size * 8; i++)
         {
             char buf2[16];

             if (value(i))
             {
                 int c = sprintf_s(buf2, ARRAY_SIZE(buf2), "%2d ", i);
                 strncat_s(buf, remaining, buf2, ARRAY_SIZE(buf2));
                 remaining -= c;
             }
         }
         strncat_s(buf, remaining, ")", 1);
     }

     bool
         GameInput::equal(GameInput& other, bool bitsonly)
     {
         if (not bitsonly and frame != other.frame)
         {
             logger->LogAndPrint(format("frames don't match: {}, {}", frame, other.frame), "game_input.cpp", "info");
         }
         if (size != other.size)
         {
             logger->LogAndPrint(format("sizes don't match: {}, {}", size, other.size), "game_input.cpp", "info");
         }
         if (memcmp(bits, other.bits, size))
         {
             logger->LogAndPrint("bits don't match", "game_input.cpp", "info");
         }

         ASSERT(size and other.size);

         return
             (bitsonly or frame == other.frame) and
             size == other.size and
             memcmp(bits, other.bits, size) == 0
             ;
     }

}
