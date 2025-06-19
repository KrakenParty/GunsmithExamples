// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/GunsmithMultiplayerPC.h"

#include "Engine/World.h"
#include "GameFramework/GameMode.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Netcode/GSNetworkLibrary.h"
#include "UI/GunsmithMultiplayerHUD.h"
#include "UI/GunsmithMultiplayerHUDWidget.h"

void AGunsmithMultiplayerPC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, bIsLobbyOwner, Params);
}

void AGunsmithMultiplayerPC::StartGame()
{
	Server_StartGame();

	SetUIInputMode(false);
}

void AGunsmithMultiplayerPC::SetLobbyOwner(bool bLobbyOwner)
{
	if (!UGSNetworkLibrary::HasAuthority(this))
	{
		return;
	}
	
	if (bIsLobbyOwner == bLobbyOwner)
	{
		return;
	}
	
	bIsLobbyOwner = bLobbyOwner;

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, bIsLobbyOwner, this);

	OnRep_bIsLobbyOwner();
}

void AGunsmithMultiplayerPC::OnRep_bIsLobbyOwner()
{
	if (bIsLobbyOwner)
	{
		if (AGunsmithMultiplayerHUD* HUD = GetHUD<AGunsmithMultiplayerHUD>())
		{
			if (UGunsmithMultiplayerHUDWidget* HUDWidget = HUD->GetMultiplayerHUDWidget())
			{
				UWidget* FocusWidget = HUDWidget->GetDesiredFocusWidget();
				SetUIInputMode(true, FocusWidget);
			}
		}
	}
	else
	{
		SetUIInputMode(false);
	}
	
	OnLobbyOwnershipChanged.Broadcast(bIsLobbyOwner);
}

void AGunsmithMultiplayerPC::Server_StartGame_Implementation()
{
	if (AGameMode* GameMode = GetWorld()->GetAuthGameMode<AGameMode>())
	{
		GameMode->StartMatch();
	}
}
