// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/GunsmithHUD.h"
#include "GunsmithMultiplayerHUD.generated.h"

class UGunsmithMultiplayerHUDWidget;
struct FGSEquipData;

/**
 *	A simple HUD for the multiplayer game mode
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithMultiplayerHUD : public AGunsmithHUD
{
	GENERATED_BODY()

public:
	// AGunsmithHUD Begin
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	// AGunsmithHUD End

	UFUNCTION(BlueprintCallable, Category="Gunsmith")
	UGunsmithMultiplayerHUDWidget* GetMultiplayerHUDWidget() const { return MultiplayerHUDWidget; }

protected:
	// The subclass of HUD widget to spawn for the multiplayer game mode
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGunsmithMultiplayerHUDWidget> MultiplayerModeHUDWidgetType = nullptr;

	// A pointer to the spawned Multiplayer HUD widget
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UGunsmithMultiplayerHUDWidget> MultiplayerHUDWidget = nullptr;

private:
	bool bIsLobbyOwner = false;
	bool bIsInPreMatchState = false;
	
	void OnLobbyOwnershipChanged(bool bIsOwner);
	void OnMatchStateChanged(FName NewState);
	void OnGameStateSet(AGameStateBase* GameStateBase);
	void OnGameStateEquipmentChanged(const FGSEquipData& NewEquipData);
};
