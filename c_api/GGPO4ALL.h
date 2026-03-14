/*******************************************************************
 *                        Peach-E v0.0.1
 *              Created by Ranyodh Mandur - 🍑 2024
 *
 *              Licensed under the MIT License (MIT).
 *         For more details, see the LICENSE file or visit:
 *               https://opensource.org/licenses/MIT
 *
 *           Peach-E is a free open source game engine
********************************************************************/
#ifndef GGPO4ALL_API_C_H_
#define GGPO4ALL_API_C_H_

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stddef.h>
#include <stdint.h> //idrc about supporting C standards from before i was born lmfao, C99+ only fuck you if you use C89 still ISO standard is C11 idgaf ab ur old ahh ABI

#include "StatusCodes.h"
#include "LoggerFlags.h"

//////////////////////////////////////////////////////////// C++ --> C compatibility preprocessor defs ////////////////////////////////////////////////////////////

#ifdef __cplusplus 

    //XXX: need to have C calling convention since this'll be called by external runtimes which probably just use a C calling convention esp C#
    #if (defined(_WIN32) || defined(_WIN64)) && defined(GGPO4ALL_BUILD_DYNAMIC)
            #define PEACH_API extern "C" __declspec(dllexport)
    #elif (defined(_WIN32) || defined(_WIN64)) && !defined(GGPO4ALL_API_STATIC)
            #define PEACH_API extern "C" __declspec(dllimport)
    #else
            #define GGPO4ALL_API extern "C"
    #endif

#else //being used from C

    #include <stdbool.h> 
    #define GGPO4ALL_API //leave empty when included in C file

#endif //C++ detection

//////////////////////////////////////////////////////////// C version preprocessor defs ////////////////////////////////////////////////////////////

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
    #define C23_CONSTEXPR constexpr
#else
    #define C23_CONSTEXPR_GGPO 
#endif

//////////////////////////////////////////////////////////// Typedefs ////////////////////////////////////////////////////////////

typedef uint64_t SESSION_ID; //all nodes are only passed by ID

//////////////////////////////////////////////////////////// Core API Functions ////////////////////////////////////////////////////////////

GGPO4ALL_API C23_CONSTEXPR_GGPO GGPO4ALL_STATUS_CODE
    GGPO4ALL_StatusCodeToString(const GGPO4ALL_STATUS_CODE fp_StatusCodesize_t, size_t fp_BufferSize, char* fp_CharBuffer);

GGPO4ALL_API GGPO4ALL_STATUS_CODE
    GGPO4ALL_CreateNewSession();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif /* GGPO4ALL_API_C_H_ */