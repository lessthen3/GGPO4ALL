/************************************************************************************************************
 *                                          GGPO4ALL v0.0.1
 *              Created by Ranyodh Mandur - ✨ 2025 and GroundStorm Studios, LLC. - ✨ 2009
 *
 *                                Licensed under the MIT License (MIT).
 *                           For more details, see the LICENSE file or visit:
 *                                  https://opensource.org/licenses/MIT
 *
 *                        GGPO4ALL is a free open source rollback netcode library
************************************************************************************************************/
#ifndef GGPO4ALL_FLAGS_API_C_H_
#define GGPO4ALL_FLAGS_API_C_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef uint32_t PEACH_LOGGER_FLAGS;

    typedef enum PeachLoggerBits
    {
        // low byte is active mask
        PEACH_TRACE_LOG = (uint32_t)(1u << 0),
        PEACH_DEBUG_LOG = (uint32_t)(1u << 1),
        PEACH_INFO_LOG = (uint32_t)(1u << 2),
        PEACH_WARNING_LOG = (uint32_t)(1u << 3),
        PEACH_ERROR_LOG = (uint32_t)(1u << 4),
        PEACH_FATAL_LOG = (uint32_t)(1u << 5),

        PEACH_ALL_LOGS = PEACH_TRACE_LOG | PEACH_DEBUG_LOG | PEACH_INFO_LOG | PEACH_WARNING_LOG | PEACH_ERROR_LOG | PEACH_FATAL_LOG,

        //middle byte is flush mask
        PEACH_FLUSH_TRACE = (uint32_t)(1u << 8),
        PEACH_FLUSH_DEBUG = (uint32_t)(1u << 9),
        PEACH_FLUSH_INFO = (uint32_t)(1u << 10),
        PEACH_FLUSH_WARNING = (uint32_t)(1u << 11),
        PEACH_FLUSH_ERROR = (uint32_t)(1u << 12),
        PEACH_FLUSH_FATAL = (uint32_t)(1u << 13),

        PEACH_FLUSH_ALL = PEACH_FLUSH_TRACE | PEACH_FLUSH_DEBUG | PEACH_FLUSH_INFO | PEACH_FLUSH_WARNING | PEACH_FLUSH_ERROR | PEACH_FLUSH_FATAL,

        //high byte is aux flags
        PEACH_DONT_CREATE_DIRECTORY = (uint32_t)(1u << 16),
        PEACH_LOG_TO_ONLY_SNAPSHOT_BUFFER = (uint32_t)(1u << 17)
    } Flags;

#ifdef __cplusplus
}
#endif

#endif /* GGPO4ALL_FLAGS_API_C_H_ */