// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GunsmithConnectionInfoWidget.generated.h"

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
	TObjectPtr<UEditableText> TextWidget;

private:
	void OnSessionCreateComplete(FName SessionName, bool bIsSuccessful);
};
