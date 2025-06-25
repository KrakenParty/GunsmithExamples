// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GunsmithMultiplayerHUDWidget.generated.h"

class UGunsmithActivatableWidget;
class UGunsmithTextDisplayWidget;
struct FGSEquipData;

/**
 *	The base HUD widget for the multiplayer game mode
 */
UCLASS()
class GUNSMITHEXAMPLES_API UGunsmithMultiplayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Displays the new equipment loadout in the text field
	void SetEquipmentText(const FGSEquipData& NewEquipData) const;

	// Called to show or hide the ownership widget depending on game state
	void UpdateLobbyOwnerWidgetVisibility(bool bShow) const;
	
protected:
	// UUserWidget Begin
	virtual bool Initialize() override;
	// UUserWidget End

	// The widget that displays for lobby owners to start the match
	UPROPERTY(EditDefaultsOnly, Category="Multiplayer Widget", meta=(BindWidget))
	TObjectPtr<UGunsmithActivatableWidget> LobbyOwnerWidget;

	// The text display widget to display current game state to players
	UPROPERTY(EditDefaultsOnly, Category="Multiplayer Widget", meta=(BindWidget))
	TObjectPtr<UGunsmithTextDisplayWidget> TextDisplayWidget;
};
