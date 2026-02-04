#include "Modules/ModuleManager.h"

class MRStoreModule : public IModuleInterface
{
public:
	virtual void StartupModule() override {}
	virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(MRStoreModule, MRStore)
