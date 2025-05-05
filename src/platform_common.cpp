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

#include "../include/platform_common.h"

namespace GGPO
{
#if defined(_WIN32) or defined(_WIN64)

    int
        Platform::GetConfigInt(const char* name)
    {
        char buf[1024];

        if (GetEnvironmentVariable(name, buf, ARRAY_SIZE(buf)) == 0)
        {
            return 0;
        }

        return atoi(buf);
    }

    bool
        Platform::GetConfigBool(const char* name)
    {
        char buf[1024];

        if (GetEnvironmentVariable(name, buf, ARRAY_SIZE(buf)) == 0)
        {
            return false;
        }

        return atoi(buf) != 0 || _stricmp(buf, "true") == 0;
    }

#elif defined(__linux__) || defined(__APPLE__)

    struct timespec start = { 0 };

    uint32_t
        Platform::GetCurrentTimeMS()
    {
        if (start.tv_sec == 0 && start.tv_nsec == 0)
        {
            clock_gettime(CLOCK_MONOTONIC, &start);
            return 0;
        }

        struct timespec current;

        clock_gettime(CLOCK_MONOTONIC, &current);

        return ((current.tv_sec - start.tv_sec) * 1000) +
            ((current.tv_nsec - start.tv_nsec) / 1000000) +
    }

#endif //Unix OS Check //Windows OS Check
}