#include <libkern/libkern.h>
#include <mach/mach_types.h>
#include <i386/proc_reg.h>
#include <i386/cpuid.h>
#include <IOKit/IOLib.h>

#ifndef MSR_IA32_POWER_CTL
#define MSR_IA32_POWER_CTL 0x1FC
#endif
#ifndef MSR_IA32_PACKAGE_THERM_STATUS
#define MSR_IA32_PACKAGE_THERM_STATUS 0x1B1
#endif
#ifndef MSR_IA32_THERM_STATUS // per core
#define MSR_IA32_THERM_STATUS 0x19C
#endif
#define MSR_IA32_THERM_STATUS_MASK 0x28A  // 0b1010001010

#ifdef __cplusplus
extern "C" {
#endif

// https://github.com/apple/darwin-xnu/blob/main/osfmk/i386/mp.h
// Perform actions on all processor cores.
extern void mp_rendezvous_no_intrs(void (*func)(void *), void *arg);

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

static void log_prochot(void* data)
{
    uint64_t mask = *((uint64_t *)data);
    uint64_t old_status = rdmsr64(MSR_IA32_THERM_STATUS);
    uint64_t new_status = old_status & ~mask;

    if (old_status & mask) {
        wrmsr64(MSR_IA32_THERM_STATUS, new_status);
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

static kern_return_t kext_start(__unused kmod_info_t *info, __unused void *data)
{
    if (!using_targeted_intel_cpu()) {
        IOLog("[GoodbyeBigSlow] Targeted Intel CPU unavailable!\n");
        return KERN_FAILURE;
    }
    IOLog("[GoodbyeBigSlow] De-asserting Processor Hot ...\n");

    mp_rendezvous_no_intrs(deassert_prochot, NULL);

    // TODO: monitoring service
    // TODO: 198H MSR_IA32_PERF_STS (IA32_PERF_STATUS)
    // TODO: 199H MSR_IA32_PERF_CTL (manually turn on)
    // TODO: SMC_Write(KPPW, Kernel_Protection_Password)
    // TODO: SMC_Write(MSAL, 0bX0XX1100)
    uint64_t status = rdmsr64(MSR_IA32_PACKAGE_THERM_STATUS);
    uint64_t mask = MSR_IA32_THERM_STATUS_MASK;
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
    mp_rendezvous_no_intrs(log_prochot, &mask);

    IOLog("[GoodbyeBigSlow] De-asserting Processor Hot ... Done\n");
    return KERN_SUCCESS;
}

static kern_return_t kext_stop(__unused kmod_info_t *info, __unused void *data)
{
    IOLog("[GoodbyeBigSlow] Stopping ...\n");
    return KERN_SUCCESS;
}

#ifdef XCODE_OFF
static kern_return_t dummy(__unused kmod_info_t *info, __unused void *data)
{
    return KERN_SUCCESS;
}
#ifndef KEXT_ID
#define KEXT_ID jakwings.kext.GoodbyeBigSlow
#endif
#ifndef KEXT_VERSION
#define KEXT_VERSION 0.0.1
#endif
#define TO_STR(x) #x
extern kern_return_t _start(kmod_info_t *, void *);
extern kern_return_t _stop(kmod_info_t *, void *);
KMOD_EXPLICIT_DECL(KEXT_ID, TO_STR(KEXT_VERSION), _start, _stop)
// NOTE: should use kext_start and kext_stop if not providing IOService
__private_extern__ kmod_start_func_t *_realmain = dummy;
__private_extern__ kmod_stop_func_t  *_antimain = dummy;
__private_extern__ int _kext_apple_cc = __APPLE_CC__;
#endif // XCODE_OFF

#ifdef __cplusplus
}
#endif // extern "C"
