// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/Tests/GunsmithGameState_Accuracy.h"

#include "GSGameplayLibrary.h"
#include "Game/GunsmithPlayerState.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Weapon/GSShootingComponent.h"

void AGunsmithGameState_Accuracy::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	TryToSetUpPlayerStateForAutoShoot(PlayerState);
}

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

void AGunsmithGameState_Accuracy::TryToSetUpPlayerStateForAutoShoot(APlayerState* Player)
{
#if !UE_BUILD_SHIPPING
	if (AutoShootData.PlayerIndex == INDEX_NONE)
	{
		return;
	}

	if (Player->GetPlayerId() == 0)
	{
		AGunsmithPlayerState* GunsmithPlayerState = Cast<AGunsmithPlayerState>(Player);

		if (ensure(GunsmithPlayerState))
		{
			GunsmithPlayerState->OnIdChanged.AddDynamic(this, &AGunsmithGameState_Accuracy::OnIdChanged);
		}
	}
	else if (Player->GetPlayerId() != AutoShootData.PlayerIndex)
	{
		APawn* Pawn = Player->GetPawn();

		if (Pawn)
		{
			if (UGSShootingComponent* ShootingComponent = UGSGameplayLibrary::GetShootingComponentFromActor(Pawn))
			{
				ShootingComponent->SetAutoShootData(AutoShootData);
			}
			else
			{
				UE_LOG(LogGunsmithTests, Warning, TEXT("Unable to find shooting component on %s"), *GetNameSafe(Pawn));
			}
		}
		else
		{
			Player->OnPawnSet.AddDynamic(this, &AGunsmithGameState_Accuracy::OnPawnSet);
		}
	}
#endif
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

void AGunsmithGameState_Accuracy::OnPawnSet(APlayerState* Player, APawn* NewPawn, APawn* OldPawn)
{
	if (Player)
	{
		Player->OnPawnSet.RemoveDynamic(this, &AGunsmithGameState_Accuracy::OnPawnSet);
	}

	TryToSetUpPlayerStateForAutoShoot(Player);
}

void AGunsmithGameState_Accuracy::OnIdChanged(APlayerState* Player)
{
	if (AGunsmithPlayerState* GunsmithPlayerState = Cast<AGunsmithPlayerState>(Player))
	{
		GunsmithPlayerState->OnIdChanged.RemoveDynamic(this, &AGunsmithGameState_Accuracy::OnIdChanged);
	}
	
	if (Player->GetPlayerId() != AutoShootData.PlayerIndex)
	{
		TryToSetUpPlayerStateForAutoShoot(Player);
	}
}
