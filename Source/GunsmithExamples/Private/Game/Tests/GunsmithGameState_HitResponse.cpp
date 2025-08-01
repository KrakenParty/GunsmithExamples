// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/Tests/GunsmithGameState_HitResponse.h"

#include "GameFramework/PlayerState.h"
#include "Netcode/GSNetworkLibrary.h"

AGunsmithGameState_HitResponse::AGunsmithGameState_HitResponse()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGunsmithGameState_HitResponse::RemovePlayerState(APlayerState* PlayerState)
{
	if (PlayerState->GetPlayerId() == AutoShootData.PlayerIndex)
	{
		SetAutoShootData(INDEX_NONE, NAME_None);
	}
	
	Super::RemovePlayerState(PlayerState);
}

void AGunsmithGameState_HitResponse::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (AutoShootData.PlayerIndex == INDEX_NONE && !PlayerArray.IsEmpty() && UGSNetworkLibrary::IsServer(this))
	{
		APlayerState* FirstPlayer = PlayerArray[0];

		if (FirstPlayer && FirstPlayer->GetPlayerId() != 0)
		{
			SetAutoShootData(0, NAME_None);
		}
	}
}
