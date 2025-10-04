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
#include <cstdint>
#include <csignal>

#define GGPO_USING_CONSOLE
#define GGPO_DEBUG

#include "../../GGPO4ALL.hpp"

static void
    SegFaultHandler(int fp_Signal)
{
    GGPO::PrintError(std::format("[!] FATAL_SEGMENTATION_FAULT, Crash signal received: {}", fp_Signal));
    // possibly notify watchdog or dump stack trace
    exit(EXIT_FAILURE);
}


int 
    main(int fp_ArgCount, const char* fp_ArgVector[])
{
    signal(SIGSEGV, SegFaultHandler); //XXX: used for trying to close and flush logs on seg fault

    GGPO::Print("UwU Rawr >O<", GGPO::Colours::BrightMagenta);

    return EXIT_SUCCESS;
}