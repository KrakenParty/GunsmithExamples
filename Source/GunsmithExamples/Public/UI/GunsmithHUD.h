// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GunsmithHUD.generated.h"

class UGunsmithHUDWidget;
/**
 *	The base HUD class for the Gunsmith Samples Project
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithHUD : public AHUD
{
	GENERATED_BODY()

public:
	// AActor Begin
	virtual void PostInitializeComponents() override;
	// AActor End

	// Get the main Gunsmith HUD widget
	UGunsmithHUDWidget* GetHUDWidget() const { return HUDWidget; }
	
protected:
	// The subclass of HUD widget to spawn
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGunsmithHUDWidget> HUDWidgetType = nullptr;

	// A pointer to the spawned HUD widget
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UGunsmithHUDWidget> HUDWidget = nullptr;

	// Bind new events if the possessed pawn changes
	UFUNCTION()
	virtual void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
};
