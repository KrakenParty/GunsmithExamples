// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/GunsmithMultiplayerGameState.h"

#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Netcode/GSNetworkLibrary.h"

void AGunsmithMultiplayerGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, CurrentRoundEquipment, Params);
}

void AGunsmithMultiplayerGameState::SetRoundEquipment(const FGSEquipData& NewEquipData)
{
	if (!UGSNetworkLibrary::HasAuthority(this))
	{
		return;
	}
	
	CurrentRoundEquipment = NewEquipData;

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, CurrentRoundEquipment, this);

	OnRep_CurrentRoundEquipment();
}

void AGunsmithMultiplayerGameState::OnRep_MatchState()
{
	Super::OnRep_MatchState();

	OnStateChangedDelegate.Broadcast(MatchState);
}

void AGunsmithMultiplayerGameState::OnRep_CurrentRoundEquipment() const
{
	OnEquipmentChangedDelegate.Broadcast(CurrentRoundEquipment);	
}
