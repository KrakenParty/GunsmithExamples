// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GunsmithHealthVignetteWidget.generated.h"

/**
 * A simple vignette that displays when health is low
 */
UCLASS()
class GUNSMITHEXAMPLES_API UGunsmithHealthVignetteWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void SetHealthViewModel(UGSHealthViewModel* HealthViewModel);	
};
