// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GunsmithMultiplayerHUDWidget.generated.h"

class UGunsmithTextDisplayWidget;
struct FGSEquipData;

/**
 *	The base HUD widget for the multiplayer game mode
 */
UCLASS()
class GUNSMITHEXAMPLES_API UGunsmithMultiplayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// UUserWidget Begin
	virtual bool Initialize() override;
	// UUserWidget End

	// The widget that displays for lobby owners to start the match
	UPROPERTY(EditDefaultsOnly, Category="Multiplayer Widget", meta=(BindWidget))
	TObjectPtr<UUserWidget> LobbyOwnerWidget;

	// The text display widget to display current game state to players
	UPROPERTY(EditDefaultsOnly, Category="Multiplayer Widget", meta=(BindWidget))
	TObjectPtr<UGunsmithTextDisplayWidget> TextDisplayWidget;
	
private:
	bool bIsLobbyOwner = false;
	bool bIsInPreMatchState = false;
	
	void OnLobbyOwnershipChanged(bool bIsOwner);
	void OnMatchStateChanged(FName NewState);
	void OnGameStateSet(AGameStateBase* GameStateBase);
	void OnGameStateEquipmentChanged(const FGSEquipData& NewEquipData);

	// Called to show or hide the ownership widget depending on game state
	void UpdateLobbyOwnerWidgetVisibility() const;
};
