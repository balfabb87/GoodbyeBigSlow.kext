#include <IOKit/IOService.h>

class GoodbyeBigSlow : public IOService
{
OSDeclareDefaultStructors(GoodbyeBigSlow)
public:
    virtual bool init(OSDictionary* personality = 0) override;
    virtual void free(void) override;
    virtual IOService* probe(IOService* provider, SInt32* score) override;
    virtual bool start(IOService* provider) override;
    virtual void stop(IOService* provider) override;
};

class GoodbyeBigSlow_NoHardPLimits : public IOService
{
OSDeclareDefaultStructors(GoodbyeBigSlow_NoHardPLimits)
public:
    virtual bool init(OSDictionary* personality = 0) override;
    virtual IOService* probe(IOService* provider, SInt32* score) override;
};
