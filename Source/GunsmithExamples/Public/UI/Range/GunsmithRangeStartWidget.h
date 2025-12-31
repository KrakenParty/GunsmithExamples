// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GunsmithRangeStartWidget.generated.h"

class URichTextBlock;

/**
 *	A simple widget to display instructions on how to start or stop the range practise
 */
UCLASS()
class GUNSMITHEXAMPLES_API UGunsmithRangeStartWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Start Widget", meta=(BindWidget))
	TObjectPtr<URichTextBlock> TextWidget;

	UPROPERTY(EditDefaultsOnly, Category="Start Widget")
	FText InactiveTextPC = FText();

	UPROPERTY(EditDefaultsOnly, Category="Start Widget")
	FText ActiveTextPC = FText();

	UPROPERTY(EditDefaultsOnly, Category="Start Widget")
	FText InactiveTextGamepad = FText();

	UPROPERTY(EditDefaultsOnly, Category="Start Widget")
	FText ActiveTextGamepad = FText();

	void OnPractiseActivityChanged(bool bIsActive) const;

	UFUNCTION()
	void OnDeviceChanged(bool bIsUsingGamepad);
	
	void SetupWorld(AGameStateBase* GameState);
};
