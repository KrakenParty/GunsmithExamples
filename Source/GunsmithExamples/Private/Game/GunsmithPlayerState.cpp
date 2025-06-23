// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/GunsmithPlayerState.h"

void AGunsmithPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();

	OnNameChanged.Broadcast(GetPlayerName());
}
