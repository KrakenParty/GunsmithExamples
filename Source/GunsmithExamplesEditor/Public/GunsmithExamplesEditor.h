#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class IAssetTypeActions;

class FGunsmithExamplesEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    TWeakPtr<IAssetTypeActions> LyraContextEffectsLibraryAssetAction;
};
