// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/GunsmithHUD.h"
#include "GunsmithMultiplayerHUD.generated.h"

class UGunsmithMultiplayerHUDWidget;

/**
 *	A simple HUD for the multiplayer game mode
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithMultiplayerHUD : public AGunsmithHUD
{
	GENERATED_BODY()

public:
	// AGunsmithHUD Begin
	virtual void PostInitializeComponents() override;
	// AGunsmithHUD End

protected:
	// The subclass of HUD widget to spawn for the multiplayer game mode
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGunsmithMultiplayerHUDWidget> MultiplayerModeHUDWidgetType = nullptr;

	// A pointer to the spawned Multiplayer HUD widget
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UGunsmithMultiplayerHUDWidget> MultiplayerHUDWidget = nullptr;
};
