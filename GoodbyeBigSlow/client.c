#include <IOKit/IOKitLib.h>
#include <Kernel/i386/proc_reg.h>

#ifndef MSR_IA32_PERF_STS
#define MSR_IA32_PERF_STS 0x198
#endif
#ifndef MSR_IA32_PERF_CTL
#define MSR_IA32_PERF_CTL 0x199
#endif
#ifndef MSR_IA32_PACKAGE_THERM_STATUS
#define MSR_IA32_PACKAGE_THERM_STATUS 0x1B1
#endif
#ifndef MSR_IA32_POWER_CTL
#define MSR_IA32_POWER_CTL 0x1FC
#endif

// TODO: class GoodbyeBigSlowClient : public IOUserClient
