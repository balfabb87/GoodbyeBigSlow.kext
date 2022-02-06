/*
 * Copyright (c) 2012 Adam Strzelecki
 *                   <https://github.com/nanoant/DisableTurboBoost.kext>
 * Copyright (c) 2015 Bernardo Alecrim
 *                   <https://github.com/balecrim/NoBatteryNoProblem.kext>
 * Copyright (c) 2022 Jak.W
 *                   <https://github.com/jakwings/GoodbyeBigSlow.kext>
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

#include <IOKit/IOLib.h>
#include "GoodbyeBigSlow.hpp"

OSDefineMetaClassAndStructors(GoodbyeBigSlow, IOService)

#include <mach/mach_types.h>
#include <libkern/libkern.h>
#include <i386/proc_reg.h>

extern "C" {
    // https://github.com/apple/darwin-xnu/blob/main/osfmk/i386/mp.h
    extern void mp_rendezvous_no_intrs(void (*action_func)(void *), void *arg);
}

const uint32_t kMsrTurbo = MSR_IA32_MISC_ENABLE;
const uint32_t kMsrProchot = 0x1FC;

// https://www.techpowerup.com/download/techpowerup-throttlestop/
const uint64_t kEnableProcHot     = 0x0000000000000001ULL;
const uint64_t kDisableTurboBoost = 0x0000004000000000ULL;

static void disable_tb_ph(__unused void* data)
{
    uint64_t msr_turbo = rdmsr64(kMsrTurbo);
    uint64_t msr_prochot = rdmsr64(kMsrProchot);
    uint64_t turbo = msr_turbo | kDisableTurboBoost;
    uint64_t prochot = msr_prochot & ~kEnableProcHot;
    IOLog("[GoodbyeBigSlow] Disabling Turbo Boost and Bi-directional Processor Hot\n");
    IOLog("[GoodbyeBigSlow] msr_turbo   : %016llx -> %016llx\n", msr_turbo, turbo);
    IOLog("[GoodbyeBigSlow] msr_prochot : %016llx -> %016llx\n", msr_prochot, prochot);
    wrmsr64(kMsrTurbo, turbo);
    wrmsr64(kMsrProchot, prochot);
}

static void enable_tb_ph(__unused void* data)
{
    uint64_t msr_turbo = rdmsr64(kMsrTurbo);
    uint64_t msr_prochot = rdmsr64(kMsrProchot);
    uint64_t turbo = msr_turbo & ~kDisableTurboBoost;
    uint64_t prochot = msr_prochot | kEnableProcHot;
    IOLog("[GoodbyeBigSlow] Enabling Turbo Boost and Bi-directional Processor Hot\n");
    IOLog("[GoodbyeBigSlow] msr_turbo   : %016llx -> %016llx\n", msr_turbo, turbo);
    IOLog("[GoodbyeBigSlow] msr_prochot : %016llx -> %016llx\n", msr_prochot, prochot);
    wrmsr64(kMsrTurbo, turbo);
    wrmsr64(kMsrProchot, prochot);
}

#define super IOService

bool GoodbyeBigSlow::init(OSDictionary* dict)
{
    IOLog("[GoodbyeBigSlow] Initializing ...\n");
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

bool GoodbyeBigSlow::start(IOService *provider)
{
    IOLog("[GoodbyeBigSlow] Starting ...\n");
    const bool result = super::start(provider);

    if (result) {
        mp_rendezvous_no_intrs(disable_tb_ph, NULL);
        IOLog("[GoodbyeBigSlow] Starting ... Success\n");
    } else {
        IOLog("[GoodbyeBigSlow] Starting ... Failure\n");
    }
    return result;
}

void GoodbyeBigSlow::stop(IOService *provider)
{
    IOLog("[GoodbyeBigSlow] Stopping ...\n");
    super::stop(provider);
    mp_rendezvous_no_intrs(enable_tb_ph, NULL);
    IOLog("[GoodbyeBigSlow] Stopping ... Done\n");
}
