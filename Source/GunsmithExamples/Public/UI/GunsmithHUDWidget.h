// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GunsmithHUDWidget.generated.h"

class UGunsmithHealthVignetteWidget;
class UGunsmithDamageIndicatorWidget;
class UGSWeaponViewModel;
class UGSHealthViewModel;
class AGunsmithMoverCharacter;

/**
 *	An example HUD widget using Gunsmith view models
 */
UCLASS()
class GUNSMITHEXAMPLES_API UGunsmithHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetupForCharacter(const AGunsmithMoverCharacter* Character);
	
	UFUNCTION(BlueprintImplementableEvent)
	void SetHealthViewModel(UGSHealthViewModel* HealthViewModel);

	UFUNCTION(BlueprintImplementableEvent)
	void SetWeaponViewModel(UGSWeaponViewModel* WeaponViewModel);

protected:
	// The widget that displays for lobby owners to start the match
	UPROPERTY(EditDefaultsOnly, Category="Gunsmith", meta=(BindWidget))
	TObjectPtr<UGunsmithDamageIndicatorWidget> DamageIndicatorWidget;

	// The widget that displays for lobby owners to start the match
	UPROPERTY(EditDefaultsOnly, Category="Gunsmith", meta=(BindWidget))
	TObjectPtr<UGunsmithHealthVignetteWidget> HealthVignetteWidget;
};
