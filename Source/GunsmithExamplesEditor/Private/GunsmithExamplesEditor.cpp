#include "GunsmithExamplesEditor.h"

#include "AssetTypeActions_LyraContextEffectsLibrary.h"

#define LOCTEXT_NAMESPACE "FGunsmithExamplesEditorModule"

void FGunsmithExamplesEditorModule::StartupModule()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	TSharedRef<FAssetTypeActions_LyraContextEffectsLibrary> AssetAction = MakeShared<FAssetTypeActions_LyraContextEffectsLibrary>();
	LyraContextEffectsLibraryAssetAction = AssetAction;
	AssetTools.RegisterAssetTypeActions(AssetAction);
}

void FGunsmithExamplesEditorModule::ShutdownModule()
{
    
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FGunsmithExamplesEditorModule, GunsmithExamplesEditor)