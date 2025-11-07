// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/Modes/Multiplayer/GunsmithMultiplayerPC.h"

#include "GSLog.h"
#include "Engine/World.h"
#include "Game/Modes/Multiplayer/GunsmithMultiplayerGameMode.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Netcode/GSNetworkLibrary.h"
#include "UI/GunsmithMultiplayerHUD.h"
#include "UI/GunsmithMultiplayerHUDWidget.h"

static FAutoConsoleCommandWithWorldAndArgs FCmdGunsmithSetMultiplayerWeapon
(
	TEXT("Gunsmith.Multiplayer.SetWeapon"),
	TEXT("Prints out all current pooling requirements"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, const UWorld* InWorld)
	{
		if (Args.Num() == 0)
		{
			UE_LOG(LogGunsmith, Log, TEXT("Unable to send an empty weapon string. Make sure the first arg is the Unique Tag of the Weapon in the data."))
			return;
		}
		
		if (AGunsmithMultiplayerPC* MultiplayerPC = Cast<AGunsmithMultiplayerPC>(UGameplayStatics::GetPlayerController(InWorld, 0)))
		{
			MultiplayerPC->Server_SetWeapon(Args[0]);
		}
	}),
	ECVF_Cheat
);

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

void AGunsmithMultiplayerPC::Server_SetWeapon_Implementation(const FString& WeaponString)
{
	const FGameplayTag WeaponTag = FGameplayTag::RequestGameplayTag(FName(WeaponString));

	if (!WeaponTag.IsValid())
	{
		return;
	}
	
	if (AGunsmithMultiplayerGameMode* GameMode = GetWorld()->GetAuthGameMode<AGunsmithMultiplayerGameMode>())
	{
		GameMode->RestartRound(WeaponTag);
	}
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
