/*
 * Copyright (c) 2012 Adam Strzelecki
 *                    https://github.com/nanoant/DisableTurboBoost.kext
 * Copyright (c) 2015 Bernardo Alecrim
 *                    https://github.com/balecrim/NoBatteryNoProblem.kext
 * Copyright (c) 2022 Jak.W
 *                    https://github.com/jakwings/GoodbyeBigSlow.kext
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <mach/mach_types.h>
#include <libkern/libkern.h>
#include <i386/proc_reg.h>
#include <i386/cpuid.h>

#ifndef MSR_IA32_POWER_CTL
#define MSR_IA32_POWER_CTL 0x1FC
#endif
#ifndef MSR_IA32_PACKAGE_THERM_STATUS // per core: 19CH IA32_THERM_STATUS
#define MSR_IA32_PACKAGE_THERM_STATUS 0x1B1
#endif

extern "C" {
    // https://github.com/apple/darwin-xnu/blob/main/osfmk/i386/mp.h
    // Perform actions on all processor cores.
    extern void mp_rendezvous_no_intrs(void (*func)(void *), void *arg);
}

// Credit to https://www.techpowerup.com/download/techpowerup-throttlestop/
const uint64_t kEnableProcHot = 0x0000000000000001ULL;

// ALERT: Toggling PROCHOT more than once in ~2 ms period can result in
//        constant Pn state (Low Frequency Mode) of the processor.
static void deassert_prochot(__unused void* data)
{
    uint64_t old_prochot = rdmsr64(MSR_IA32_POWER_CTL);
    uint64_t new_prochot = old_prochot & ~kEnableProcHot;

    // hopefully not to cause invalid write and the black screen of death
    if (old_prochot & kEnableProcHot) {
        wrmsr64(MSR_IA32_POWER_CTL, new_prochot);
    }
}

static bool using_targeted_intel_cpu(void)
{
    uint32_t registers[4] = {[eax]=0x00, [ebx]=0xFF, [ecx]=0xFF, [edx]=0xFF};
    cpuid(registers);
    uint32_t maxval = registers[eax];

    bool GenuineIntel = maxval >= 0x01 &&
                        registers[ebx] == 0x756E6547 &&
                        registers[edx] == 0x49656E69 &&
                        registers[ecx] == 0x6C65746E;

    // check cpu vendor
    if (GenuineIntel) {
        registers[eax] = 0x01;
        cpuid(registers);
        // check cpu family but not model
        if (((registers[eax] >> 8) & 0x0F) == 0x06) {
            // supports package thermal management (PTM) ?
            if (maxval >= 0x06) {
                registers[eax] = 0x06;
                cpuid(registers);
                return registers[eax] & (1 << 6);
            }
        }
    }
    return false;
}

/*==========================================================================*\
)   Virtual Driver auto loaded at boot time.                                 (
\*==========================================================================*/

#include <IOKit/IOLib.h>
#include "GoodbyeBigSlow.hpp"

OSDefineMetaClassAndStructors(GoodbyeBigSlow, IOService)

#define super IOService

bool GoodbyeBigSlow::init(OSDictionary* dict)
{
    IOLog("[GoodbyeBigSlow] Initializing ...\n");

    if (!using_targeted_intel_cpu()) {
        IOLog("[GoodbyeBigSlow] Targeted Intel CPU unavailable!\n");
        IOLog("[GoodbyeBigSlow] Initializing ... Failure\n");
        return false;
    }

    const auto result = super::init(dict);

    if (result) {
        IOLog("[GoodbyeBigSlow] Initializing ... Success\n");
    } else {
        IOLog("[GoodbyeBigSlow] Initializing ... Failure\n");
    }
    return result;
}

void GoodbyeBigSlow::free(void)
{
    IOLog("[GoodbyeBigSlow] Freeing ...\n");
    super::free();
    IOLog("[GoodbyeBigSlow] Freeing ... Done\n");
}

IOService* GoodbyeBigSlow::probe(IOService* provider, SInt32* score)
{
    IOLog("[GoodbyeBigSlow] Probing ...\n");
    const auto result = super::probe(provider, score);

    if (result) {
        IOLog("[GoodbyeBigSlow] Probing ... Success\n");
    } else {
        IOLog("[GoodbyeBigSlow] Probing ... Failure\n");
    }
    return result;
}

bool GoodbyeBigSlow::start(IOService* provider)
{
    IOLog("[GoodbyeBigSlow] Starting ...\n");
    const auto result = super::start(provider);

    if (result) {
        IOLog("[GoodbyeBigSlow] De-asserting Processor Hot ...\n");
        // TODO: monitoring service
        {
            mp_rendezvous_no_intrs(deassert_prochot, NULL);

            // TODO: 198H MSR_IA32_PERF_STS (IA32_PERF_STATUS)
            // TODO: 199H MSR_IA32_PERF_CTL
            uint64_t status = rdmsr64(MSR_IA32_PACKAGE_THERM_STATUS);
            uint64_t mask = 0x28A;  // 0b1010001010
            if (status & (1 << 11)) {
                uint32_t registers[4] = {[eax]=6, [ebx]=0, [ecx]=0, [edx]=0};
                cpuid(registers);
                if (registers[eax] & (1 << 4)) {
                    mask |= (1 << 11);
                }
            }
            if (status & mask) {
                status &= ~mask;  // clear PROCHOT logs
                wrmsr64(MSR_IA32_PACKAGE_THERM_STATUS, status);
            }
        }
        IOLog("[GoodbyeBigSlow] De-asserting Processor Hot ... Done\n");
        IOLog("[GoodbyeBigSlow] Starting ... Success\n");
    } else {
        IOLog("[GoodbyeBigSlow] Starting ... Failure\n");
    }
    return result;
}

void GoodbyeBigSlow::stop(IOService* provider)
{
    IOLog("[GoodbyeBigSlow] Stopping ...\n");
    super::stop(provider);
    IOLog("[GoodbyeBigSlow] Stopping ... Done\n");
}
