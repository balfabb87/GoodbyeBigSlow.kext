/*==========================================================================*\
)   Virtual Driver auto loaded at boot time.                                 (
\*==========================================================================*/

#include <IOKit/IOLib.h>
#include "GoodbyeBigSlow.c"
#include "GoodbyeBigSlow.hpp"

OSDefineMetaClassAndStructors(GoodbyeBigSlow, IOService)
OSDefineMetaClassAndStructors(GoodbyeBigSlow_NoHardPLimits, IOService)

#define super IOService

bool GoodbyeBigSlow::init(OSDictionary* personality)
{
    DBLogStatus("Initializing", -1);
    const auto result = super::init(personality);
    DBLogStatus("Initializing", result ? 1 : 0);
    return result;
}

void GoodbyeBigSlow::free(void)
{
    DBLog("Freeing ...");
    super::free();
}

// Driver Matching: 1. IOProviderClass -> 2. personality -> 3. IOProbeScore
// Step 3: init() attach() probe() detach() // free() if probe fails
IOService* GoodbyeBigSlow::probe(IOService* provider, SInt32* score)
{
    DBLogStatus("Probing", -1);
    const auto result = super::probe(provider, score);
    DBLogStatus("Probing", result ? 1 : 0);
    return result;
}

bool GoodbyeBigSlow::start(IOService* provider)
{
    DBLogStatus("Starting", -1);
    const auto result = kext_start(NULL, NULL) == KERN_SUCCESS && super::start(provider);
    DBLogStatus("Starting", result ? 1 : 0);
    return result;
}

void GoodbyeBigSlow::stop(IOService* provider)
{
    kext_stop(NULL, NULL);
    super::stop(provider);
}
