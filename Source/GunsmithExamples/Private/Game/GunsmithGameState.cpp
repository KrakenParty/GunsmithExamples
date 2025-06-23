// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/GunsmithGameState.h"

DEFINE_LOG_CATEGORY(LogGunsmithTests);

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
