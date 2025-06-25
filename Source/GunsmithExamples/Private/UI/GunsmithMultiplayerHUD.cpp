// Copyright Kraken Party Limited. All Rights Reserved.

#include "UI/GunsmithMultiplayerHUD.h"

#include "Blueprint/UserWidget.h"
#include "Game/Modes/Multiplayer/GunsmithMultiplayerGameState.h"
#include "Game/Modes/Multiplayer/GunsmithMultiplayerPC.h"
#include "GameFramework/GameMode.h"
#include "UI/GunsmithHUDWidget.h"
#include "UI/GunsmithMultiplayerHUDWidget.h"

void AGunsmithMultiplayerHUD::BeginPlay()
{
	Super::BeginPlay();

	if (AGunsmithMultiplayerPC* Controller = Cast<AGunsmithMultiplayerPC>(PlayerOwner))
	{
		Controller->OnLobbyOwnershipChanged.AddUObject(this, &AGunsmithMultiplayerHUD::OnLobbyOwnershipChanged);
		OnLobbyOwnershipChanged(Controller->GetIsLobbyOwner());
	}

	UWorld* World = GetWorld();
	if (AGunsmithMultiplayerGameState* GameState =  World->GetGameState<AGunsmithMultiplayerGameState>())
	{
		OnGameStateSet(GameState);
	}
	else
	{
		World->GameStateSetEvent.AddUObject(this, &AGunsmithMultiplayerHUD::OnGameStateSet);
	}
}

void AGunsmithMultiplayerHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (MultiplayerModeHUDWidgetType)
	{
		MultiplayerHUDWidget = CreateWidget<UGunsmithMultiplayerHUDWidget>(PlayerOwner, MultiplayerModeHUDWidgetType, TEXT("MultiplayerHUD"));

		if (MultiplayerHUDWidget)
		{
			MultiplayerHUDWidget->AddToViewport();
		}
	}
}

void AGunsmithMultiplayerHUD::OnLobbyOwnershipChanged(bool bIsOwner)
{
	bIsLobbyOwner = bIsOwner;

	MultiplayerHUDWidget->UpdateLobbyOwnerWidgetVisibility(bIsLobbyOwner && bIsInPreMatchState);
}

void AGunsmithMultiplayerHUD::OnMatchStateChanged(FName NewState)
{
	bIsInPreMatchState = NewState == MatchState::WaitingToStart;

	HUDWidget->SetVisibility(bIsInPreMatchState ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	MultiplayerHUDWidget->UpdateLobbyOwnerWidgetVisibility(bIsLobbyOwner && bIsInPreMatchState);
}

void AGunsmithMultiplayerHUD::OnGameStateSet(AGameStateBase* GameStateBase)
{
	if (AGunsmithMultiplayerGameState* GameState =  Cast<AGunsmithMultiplayerGameState>(GameStateBase))
	{
		GameState->OnStateChangedDelegate.AddUObject(this, &AGunsmithMultiplayerHUD::OnMatchStateChanged);
		GameState->OnEquipmentChangedDelegate.AddUObject(this, &AGunsmithMultiplayerHUD::OnGameStateEquipmentChanged);
		OnMatchStateChanged(GameState->GetMatchState());
	}
}

void AGunsmithMultiplayerHUD::OnGameStateEquipmentChanged(const FGSEquipData& NewEquipData)
{
	MultiplayerHUDWidget->SetEquipmentText(NewEquipData);
}
