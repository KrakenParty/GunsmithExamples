// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GunsmithHUD.generated.h"

class UGunsmithHUDWidget;
class UGunsmithPauseMenuWidget;
class UWidget;

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
	UFUNCTION(BlueprintCallable, Category="Gunsmith")
	UGunsmithHUDWidget* GetHUDWidget() const { return HUDWidget; }

	// Sets the HUD to the paused state. Returns a focus widget if one exists
	UWidget* SetPaused(bool bPaused) const;
	
protected:
	// The subclass of HUD widget to spawn
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGunsmithHUDWidget> HUDWidgetType = nullptr;

	// A pointer to the spawned HUD widget
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UGunsmithHUDWidget> HUDWidget = nullptr;

	// The subclass of pause menu widget to spawn
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGunsmithPauseMenuWidget> PauseMenuWidgetType = nullptr;

	// A pointer to the spawned pause menu widget
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UGunsmithPauseMenuWidget> PauseMenuWidget = nullptr;

	// Bind new events if the possessed pawn changes
	UFUNCTION()
	virtual void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
};
