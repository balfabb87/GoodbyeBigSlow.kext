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

// ALERT: Toggling PROCHOT more than once in ~2 ms period will result in
//        constant Pn state of the processor.
// https://www.techpowerup.com/download/techpowerup-throttlestop/
const uint32_t kMsrProchot = 0x1FC;
const uint64_t kEnableProcHot = 0x0000000000000001ULL;

static void deassert_prochot(__unused void* data)
{
    uint64_t old_prochot = rdmsr64(kMsrProchot);
    uint64_t new_prochot = old_prochot & ~kEnableProcHot;
    IOLog("[GoodbyeBigSlow] De-asserting Processor Hot ...\n");
    IOLog("[GoodbyeBigSlow] msr_prochot : %016llx -> %016llx\n", old_prochot, new_prochot);
    wrmsr64(kMsrProchot, new_prochot);
    IOLog("[GoodbyeBigSlow] De-asserting Processor Hot ... Done\n");
}

static void assert_prochot(__unused void* data)
{
    uint64_t old_prochot = rdmsr64(kMsrProchot);
    uint64_t new_prochot = old_prochot | kEnableProcHot;
    IOLog("[GoodbyeBigSlow] Asserting Processor Hot ...\n");
    IOLog("[GoodbyeBigSlow] msr_prochot : %016llx -> %016llx\n", old_prochot, new_prochot);
    wrmsr64(kMsrProchot, new_prochot);
    IOLog("[GoodbyeBigSlow] Asserting Processor Hot ... Done\n");
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

bool GoodbyeBigSlow::start(IOService* provider)
{
    IOLog("[GoodbyeBigSlow] Starting ...\n");
    const bool result = super::start(provider);

    if (result) {
        mp_rendezvous_no_intrs(deassert_prochot, NULL);
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
    mp_rendezvous_no_intrs(assert_prochot, NULL);
    IOLog("[GoodbyeBigSlow] Stopping ... Done\n");
}
