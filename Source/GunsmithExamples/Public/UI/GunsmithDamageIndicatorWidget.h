// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GunsmithDamageIndicatorWidget.generated.h"

class UGSHealthViewModel;
class AGunsmithMoverCharacter;

/**
 *	An example damage indicator widget
 */
UCLASS()
class GUNSMITHEXAMPLES_API UGunsmithDamageIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetupForCharacter(const AGunsmithMoverCharacter* Character);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void SetHealthViewModel(UGSHealthViewModel* HealthViewModel);
};
