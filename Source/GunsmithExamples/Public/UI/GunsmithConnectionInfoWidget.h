// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GunsmithConnectionInfoWidget.generated.h"

class UTextBlock;
class UEditableText;

/**
 *	A simple widget to display the local players connection info to make it easier to coordinate multiplayer games
 */
UCLASS()
class GUNSMITHEXAMPLES_API UGunsmithConnectionInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// UUserWidget Begin
	virtual void NativeOnInitialized() override;
	// UUserWidget End

protected:
	UPROPERTY(EditDefaultsOnly, Category="Gunsmith", meta=(BindWidget))
	TObjectPtr<UTextBlock> SessionTypeText;
	
	UPROPERTY(EditDefaultsOnly, Category="Gunsmith", meta=(BindWidget))
	TObjectPtr<UEditableText> TextWidget;

	UPROPERTY(EditDefaultsOnly, Category="Gunsmith", meta=(BindWidget))
	TObjectPtr<UTextBlock> JoiningInfoText;
	
private:
	bool bIsInitialized = false;
	
	void OnSessionCreateComplete(FName SessionName, bool bIsSuccessful);
};
