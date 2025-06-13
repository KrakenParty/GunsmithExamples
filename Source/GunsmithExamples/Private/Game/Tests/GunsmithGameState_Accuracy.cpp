// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/Tests/GunsmithGameState_Accuracy.h"

#include "GSGameplayLibrary.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Weapon/GSShootingComponent.h"

void AGunsmithGameState_Accuracy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, AutoShootData, Params);
}

void AGunsmithGameState_Accuracy::SetAutoShootData(int32 Index, FName BoneName)
{
	if (Index != INDEX_NONE && !PlayerArray.IsValidIndex(Index))
	{
		UE_LOG(LogGunsmithTests, Log, TEXT("Unable to start auto shoot as player state with ID %d does not exist"), Index);
		return;
	}
	
	AutoShootData.PlayerIndex = Index == INDEX_NONE ? INDEX_NONE : PlayerArray[Index]->GetPlayerId();
	AutoShootData.BoneName = BoneName;

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, AutoShootData, this);
}

void AGunsmithGameState_Accuracy::OnRep_AutoShootData()
{
#if !UE_BUILD_SHIPPING
	for (APlayerState* PlayerState : PlayerArray)
	{
		if (!PlayerState || PlayerState->GetPlayerId() == AutoShootData.PlayerIndex)
		{
			continue;
		}
		
		if (UGSShootingComponent* ShootingComponent = UGSGameplayLibrary::GetShootingComponentFromActor(PlayerState->GetPawn()))
		{
			ShootingComponent->SetAutoShootData(AutoShootData);
		}
	}
#endif
}
