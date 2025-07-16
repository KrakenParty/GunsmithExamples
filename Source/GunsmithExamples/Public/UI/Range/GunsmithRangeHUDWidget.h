// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GunsmithRangeHUDWidget.generated.h"

class UGunsmithRangeScoreWidget;
class UGunsmithRangeStartWidget;

/**
 *	A widget containing all other widgets required for the range mode
 */
UCLASS()
class GUNSMITHEXAMPLES_API UGunsmithRangeHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category="Range Widget", meta=(BindWidget))
	TObjectPtr<UGunsmithRangeScoreWidget> ScoreWidget;

	UPROPERTY(EditDefaultsOnly, Category="Range Widget", meta=(BindWidget))
	TObjectPtr<UGunsmithRangeStartWidget> StartWidget;
};
