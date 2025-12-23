// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/GunsmithGameState.h"

#include "Engine/GameInstance.h"
#include "Weapon/GSWeaponsSubsystem.h"

DEFINE_LOG_CATEGORY(LogGunsmithTests);

void AGunsmithGameState::BeginPlay()
{
	Super::BeginPlay();

	// Start preloading assets as soon as the game state is ready
	// Other games could do this during a loading screen, before transitioning into a level
	if (PreloadData.PreloadDataTable)
	{		
		if (UGSWeaponsSubsystem* WeaponsSubsystem = GetGameInstance()->GetSubsystem<UGSWeaponsSubsystem>())
		{
			TWeakObjectPtr<AGunsmithGameState> WeakThis(this);
			WeaponsSubsystem->PreloadObjects(PreloadData, FGSWeaponsSystemPreloadCompleteDelegate::FDelegate::CreateWeakLambda(this, [WeakThis](const UGSWeaponsSubsystem* Subsystem)
			{
				if (WeakThis.Get())
				{
					WeakThis->bIsInitialized = true;
				}
			}));
		}
	}
	else
	{
		bIsInitialized = true;
	}
}

void AGunsmithGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (UGSWeaponsSubsystem* WeaponsSubsystem = GetGameInstance()->GetSubsystem<UGSWeaponsSubsystem>())
	{
		WeaponsSubsystem->ClearLoadedData();
	}
}

void AGunsmithGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	OnPlayerAdded.Broadcast(PlayerState);
}

void AGunsmithGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	OnPlayerRemoved.Broadcast(PlayerState);
}

bool AGunsmithGameState::ShouldShowLoadingScreen(FString& OutReason) const
{
	if (!bIsInitialized)
	{
		OutReason = "Waiting for GunsmithGameState to finish loading";
		return true;
	}

	return false;
}
