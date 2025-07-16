// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/GunsmithHUD.h"
#include "GunsmithRangeHUD.generated.h"

class UGunsmithRangeHUDWidget;

/**
 *	A HUD class for the shooting range which displays score details
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithRangeHUD : public AGunsmithHUD
{
	GENERATED_BODY()

public:
	// AGunsmithHUD Begin
	virtual void PostInitializeComponents() override;
	// AGunsmithHUD End
	
protected:
	// The subclass of HUD widget to spawn for the range game mode
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGunsmithRangeHUDWidget> RangeHUDWidgetType = nullptr;

	// A pointer to the spawned range HUD widget
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UGunsmithRangeHUDWidget> RangeHUDWidget = nullptr;
};
