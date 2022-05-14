#include <libkern/libkern.h>
#include <mach/mach_types.h>
#include <i386/proc_reg.h>
#include <i386/cpuid.h>
#include <sys/ioccom.h>
#include <IOKit/IOLib.h>

// IDEA: kick X86PlatformShim.kext off the candidate list (failed)
// IDEA: patch __ZN15X86PlatformShim18sendHardPLimitXCPMEi (Lilu.kext)
// IDEA: SMC_Write(KPPW, KernelProtectionPassword) SMC_Write(MSAL, 0bX0XX1100)
// TODO: GoodbyeBigSlowClient
//       $ plimit [<frequency>]
//       $ gpu frequency [<frequency>]
//       $ cpu frequency [<frequency>]
//       198H MSR_IA32_PERF_STS (IA32_PERF_STATUS; CPUID.01H:ECX[7]=1)
//       199H MSR_IA32_PERF_CTL (mask: 0xFF00 for targeted CPUs)
//       $ cpu hwp [<frequency>]
//       770H IA32_PM_ENABLE[0] (alternative; Write-Once; CPUID.06H:EAX.[7]=1)
//       $ voltage <min> <max>
//       $ msr read  <address>
//       $ msr write <address> <integer|0xHHHHHHHHHHHHHHHH>

#ifdef __cplusplus
extern "C" {
#endif

// https://developer.apple.com/documentation/kernel/1576460-osincrementatomic
// https://lwn.net/Articles/793253/
#define VOLATILE_ACCESS(x) (*((volatile __typeof__(x) *)&(x)))

#ifndef MSR_IA32_POWER_CTL
#define MSR_IA32_POWER_CTL 0x1FC
#endif
// Credit to https://www.techpowerup.com/download/techpowerup-throttlestop/
const uint64_t kMsrEnableProcHot = 1;

#ifndef MSR_IA32_MISC_ENABLE
#define MSR_IA32_MISC_ENABLE 0x1A0
#endif
const uint64_t kMsrEnableSpeedStep = 1ULL << 16;
const uint64_t kMsrDisableTurboBoost = 1ULL << 38;

#ifndef MSR_IA32_PACKAGE_THERM_STATUS
#define MSR_IA32_PACKAGE_THERM_STATUS 0x1B1
#endif
#ifndef MSR_IA32_THERM_STATUS // per core
#define MSR_IA32_THERM_STATUS 0x19C
#endif
const uint64_t kMsrThermalStatusMask = 0x28A;  // 0b1010001010

// https://github.com/apple/darwin-xnu/blob/main/bsd/sys/ioccom.h
#define XCPM_WUT _IOW('X', 0, uint32_t)  // ???
#define XCPM_GET_PSTATES _IO('X', 0x01)  // MHz step by 100MHz
// ? AICPM_SET_CPU_HARD_PLIMIT_MAX (AppleIntelCPUPowerManagement.kext) ?
#define XCPM_SET_CPU_HARD_PLIMIT_MAX _IOWR('X', 0x20, uint64_t)  // 100MHz?

#pragma pack(push, 8)
struct XCPMPstates {
    uint64_t freq_turbo;
    uint64_t freq_core;
    uint64_t freq_count;
    uint32_t data[256];  // [(Pn,min), ..., (Px,norm), ..., (P0,turbo), ...] ?
};
#pragma pack(pop)

// https://github.com/apple/darwin-xnu/blob/main/osfmk/i386/pmCPU.h
// pmDispatch->*: /System/Library/Extensions/AppleIntelCPUPowerManagement.kext
// Called to get/set CPU power management state.
extern int pmCPUControl(uint32_t cmd, void *arg);

// https://github.com/apple/darwin-xnu/blob/main/osfmk/i386/mp.h
// Perform actions on all processor cores.
extern void mp_rendezvous_no_intrs(void (*func)(void *), void *arg);

// ALERT: Toggling PROCHOT more than once in ~2 ms period can result in
//        constant Pn state (Low Frequency Mode) of the processor.
static void mp_deassert_prochot(void *data)
{
    SInt64 *cpucount = (SInt64 *)data;
    SInt64 *disabled = (SInt64 *)data + 1;
    uint64_t old_bits = rdmsr64(MSR_IA32_POWER_CTL);
    uint64_t new_bits = old_bits & ~kMsrEnableProcHot;

    // hopefully not to cause invalid write and the black screen of death
    if (old_bits != new_bits) {
        wrmsr64(MSR_IA32_POWER_CTL, new_bits);
        IOSleep(1);
        if (!(rdmsr64(MSR_IA32_POWER_CTL) & kMsrEnableProcHot)) {
            OSIncrementAtomic64(VOLATILE_ACCESS(disabled));
        }
    } else {
        OSIncrementAtomic64(VOLATILE_ACCESS(disabled));
    }
    OSIncrementAtomic64(VOLATILE_ACCESS(cpucount));
}

static void mp_log_prochot(void *data)
{
    uint64_t mask = *((uint64_t *)data);
    uint64_t old_bits = rdmsr64(MSR_IA32_THERM_STATUS);
    uint64_t new_bits = old_bits & ~mask;

    if (old_bits != new_bits) {
        wrmsr64(MSR_IA32_THERM_STATUS, new_bits);
    }
}

static void log_prochot(void)
{
    uint64_t mask = kMsrThermalStatusMask;
    uint64_t old_bits = rdmsr64(MSR_IA32_PACKAGE_THERM_STATUS);
    uint64_t new_bits = old_bits & ~mask;

    if (old_bits & (1 << 11)) {
        uint32_t registers[4] = {[eax]=6, [ebx]=0, [ecx]=0, [edx]=0};
        cpuid(registers);

        if (registers[eax] & (1 << 4)) {
            mask |= (1 << 11);
            new_bits &= ~mask;
        }
    }
    if (old_bits != new_bits) {
        wrmsr64(MSR_IA32_PACKAGE_THERM_STATUS, new_bits);
    }
    mp_rendezvous_no_intrs(mp_log_prochot, &mask);
}

static bool deassert_prochot(void)
{
    SInt64 counts[2] __attribute__((aligned(sizeof(SInt64)))) = {0, 0};

    mp_rendezvous_no_intrs(mp_deassert_prochot, counts);

    if (counts[0] == counts[1]) {
        log_prochot();
        return true;
    }
    return false;
}

static bool disable_turbo(void)
{
    uint32_t registers[4] = {[eax]=6, [ebx]=0, [ecx]=0, [edx]=0};
    cpuid(registers);

    if (registers[eax] & (1 << 1)) {
        uint64_t old_bits = rdmsr64(MSR_IA32_MISC_ENABLE);
        uint64_t new_bits = old_bits | kMsrDisableTurboBoost;

        if (old_bits != new_bits) {
            // XXX: CPUID.06H:EAX[1] => 0
            wrmsr64(MSR_IA32_MISC_ENABLE, new_bits);
            IOSleep(1);
            return rdmsr64(MSR_IA32_MISC_ENABLE) & kMsrDisableTurboBoost;
        }
    }
    return true;
}

static bool disable_speedstep(void)
{
    uint32_t registers[4] = {[eax]=1, [ebx]=0, [ecx]=0, [edx]=0};
    cpuid(registers);

    if (registers[ecx] & (1 << 7)) {
        uint64_t old_bits = rdmsr64(MSR_IA32_MISC_ENABLE);
        uint64_t new_bits = old_bits & ~kMsrEnableSpeedStep;

        if (old_bits != new_bits) {
            wrmsr64(MSR_IA32_MISC_ENABLE, new_bits);
            IOSleep(1);
            return !(rdmsr64(MSR_IA32_MISC_ENABLE) & kMsrEnableSpeedStep);
        }
    }
    return true;
}

static bool eql_flag(const char *a, const char *b, size_t n)
{
    while (n > 0 && *a && *b) {
        if (*a != *b || *a == ':' || *b == ':') return false;
        ++a; ++b; --n;
    }
    return n == 0;
}
static bool has_flag(const char *args, const char *arg)
{
    if (arg[0] == '-' || arg[0] == '+') {
        size_t n = strlen(arg);
        for (const char *p = args; *p; ++p) {
            if ((p == args || p[-1] == ':') && (p[n] == 0 || p[n] == ':')
                    && eql_flag(p, arg, n)) {
                return true;
            }
        }
    }
    return false;
}

static bool using_targeted_intel_cpu(void)
{
    uint32_t registers[4] = {[eax]=0x00, [ebx]=0xFF, [ecx]=0xFF, [edx]=0xFF};
    cpuid(registers);
    uint32_t maxval = registers[eax];

    bool GenuineIntel = registers[ebx] == 0x756E6547 &&
                        registers[edx] == 0x49656E69 &&
                        registers[ecx] == 0x6C65746E &&
                        maxval >= 0x01;

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

#define LOG(fmt, ...) IOLog("[GoodbyeBigSlow] " fmt "\n", __VA_ARGS__)

static void DBLog(const char *s)
{
    LOG("%s", s);
}

static void DBLogStatus(const char *s, int n)
{
    if (n == -1) {
        LOG("%s ...", s);
    } else {
        LOG("%s ... %s", s, n ? "Success" : "Failure");
    }
}

static kern_return_t kext_start(__unused kmod_info_t *_o, __unused void *data)
{
    if (!using_targeted_intel_cpu()) {
        DBLog("Targeted Intel CPU unavailable!");
        return KERN_FAILURE;
    }
    int ret = -1;

    // TODO: pmDispatch seems always unready (IOProbeScore=-1) need permission?
    uint32_t pmmode = 0;
    DBLog("Detecting Power Management Mode ...");
    if ((ret = pmCPUControl(XCPM_WUT, &pmmode)) == 0 && pmmode == 2) {
        DBLog("Detected Power Management Mode: XCPM");

        struct XCPMPstates states;
        memset(&states, 0, sizeof(states));

        DBLogStatus("Getting PStates", -1);
        if (pmCPUControl(XCPM_GET_PSTATES, &states) == 0) {
            DBLogStatus("Getting PStates", 1);
            LOG("Frequency = %llu-%lluMHz", states.freq_core, states.freq_turbo);
            LOG("Count = %llu", states.freq_count);
            for (uint64_t i = 0; i < states.freq_count / 2; i++) {
                LOG("(%02llu) P%u = %uMHz", i, states.data[i], states.data[i+1]);
            }

            //uint64_t limit = states.freq_turbo / 100;
            //DBLogStatus("Resetting Hard PLimits", -1);
            //ret = pmCPUControl(XCPM_SET_CPU_HARD_PLIMIT_MAX, &limit) == 0;
            //DBLogStatus("Resetting Hard PLimits", ret);
        } else {
            DBLogStatus("Getting PStates", 0);
        }
    } else {
        LOG("Detected Power Management Mode: AICPM (%d,%u) ?", ret, pmmode);
    }

    // GoodbyeBigSlow=<flags>  // "-": disable; "+": enable
#define BOOT_ARGS_SIZE sizeof("-turbo:-speedstep")
    // https://github.com/apple/darwin-xnu/blob/main/pexpert/gen/bootargs.c
    // XXX: better use own parser if this kext is needed for macos < v10.11.6
    // PE_parse_boot_argn(<key>, arg_ptr, arg_size) => matched?
    //     argument (the part before "=" is compared with <key>)
    //         boolean: (?P<key>-[^\x09\x20=]*)(=[^\x09\x20]*)?
    //         key=val: (?P<key>[^\x09\x20\-=]*)=(?P<val>[^\x09\x20]*)
    //         skipped: not -* nor *=*
    // if boot-arg is boolean
    // then *(intN_t *)arg_ptr = 1
    // else if <key> begins with "_"
    // then strlcpy(arg_ptr, <val>, max(16, arg_size - 1))  // forced & unsafe
    // else if <val> is (+/-) 0xHHHH... 0b1010... 0755... (k/K/m/M/g/G)
    // then *(intN_t *)arg_ptr = integer(<val>)  // N = max(arg_size * 8, 64)
    // else strlcpy(arg_ptr, <val>, arg_size - 1)
    // return true after the first match; no copy/assignment if arg_size is 0
    char boot_args[BOOT_ARGS_SIZE];

    // FIXME: CPU model and MSR read/write permission not checked
    if (PE_parse_boot_argn("GoodbyeBigSlow", &boot_args, BOOT_ARGS_SIZE)) {
        boot_args[BOOT_ARGS_SIZE - 1] = 0;
        if (has_flag(boot_args, "-turbo")) {
            DBLogStatus("Disabling Turbo Boost", -1);
            ret = disable_turbo();
            DBLogStatus("Disabling Turbo Boost", ret);
        }
        if (has_flag(boot_args, "-speedstep")) {
            DBLogStatus("Disabling SpeedStep", -1);
            ret = disable_speedstep();
            DBLogStatus("Disabling SpeedStep", ret);
        }
    }

    DBLogStatus("De-asserting Processor Hot", -1);
    ret = deassert_prochot();
    DBLogStatus("De-asserting Processor Hot", ret);

    return KERN_SUCCESS;
}

static kern_return_t kext_stop(__unused kmod_info_t *_o, __unused void *data)
{
    DBLog("Stopping ...");
    return KERN_SUCCESS;
}

#ifdef XCODE_OFF
static kern_return_t dummy(__unused kmod_info_t *_o, __unused void *data)
{
    return KERN_SUCCESS;
}
#ifndef KEXT_ID
#error KEXT_ID undefined
#endif
#ifndef KEXT_VERSION
#error KEXT_VERSION undefined
#endif
#define TO_STR(tokens) #tokens
extern kern_return_t _start(kmod_info_t *, void *);
extern kern_return_t _stop(kmod_info_t *, void *);
KMOD_EXPLICIT_DECL(KEXT_ID, TO_STR(KEXT_VERSION), _start, _stop)
// NOTE: should use kext_start and kext_stop if not providing IOService
__private_extern__ kmod_start_func_t *_realmain = dummy;
__private_extern__ kmod_stop_func_t  *_antimain = dummy;
__private_extern__ int _kext_apple_cc = __APPLE_CC__;
#endif // XCODE_OFF

#ifdef __cplusplus
} // extern "C"
#endif
