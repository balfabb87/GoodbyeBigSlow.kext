/*==========================================================================*\
)   Virtual Driver auto loaded at boot time.                                 (
\*==========================================================================*/

#include <IOKit/IOLib.h>
#include "GoodbyeBigSlow.c"
#include "GoodbyeBigSlow.hpp"

OSDefineMetaClassAndStructors(GoodbyeBigSlow, IOService)

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
        IOLog("[GoodbyeBigSlow] Starting ... Success\n");
        if (kext_start(NULL, NULL) != KERN_SUCCESS) {
            super::stop(provider);
            return false;
        }
    } else {
        IOLog("[GoodbyeBigSlow] Starting ... Failure\n");
    }
    return result;
}

void GoodbyeBigSlow::stop(IOService* provider)
{
    kext_stop(NULL, NULL);
    super::stop(provider);
}
