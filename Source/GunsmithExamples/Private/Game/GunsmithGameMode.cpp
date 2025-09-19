// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/GunsmithGameMode.h"

#include "GSGameplayLibrary.h"
#include "GSLog.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Health/GSHealthComponent.h"

APawn* AGunsmithGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer,
	const FTransform& SpawnTransform)
{
	APawn* Pawn =  Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);
	
	if (UGSHealthComponent* HealthComponent = UGSGameplayLibrary::GetHealthComponentFromActor(Pawn))
	{
		HealthComponent->OnDeath.AddUniqueDynamic(this, &AGunsmithGameMode::OnPawnDeath);
	}
	
	return Pawn;
}

void AGunsmithGameMode::StartRespawn(AController* Controller, float Time)
{
	if (!Controller)
	{
		return;
	}
	
	Controller->UnPossess();

	// Instantly respawn
	if (Time <= 0.0f)
	{
		RespawnPlayer(Controller, FTimerHandle());
		return;
	}
	
	// Respawn after RespawnTime seconds
	FTimerHandle TimerHandle;
				
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &AGunsmithGameMode::RespawnPlayer, Controller, TimerHandle);
				
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, Time, false);

	PendingRespawnHandles.Emplace(TimerHandle);
}

void AGunsmithGameMode::OnPawnDeath(UGSHealthComponent* HealthComponent, const FGSDamageRecord& DamageRecord, bool bIsPredicted)
{
	if (bShouldAutoRespawn)
	{
		if (APawn* OwningPawn = Cast<APawn>(HealthComponent->GetOwner()))
		{
			if (AController* Controller = OwningPawn->GetController())
			{
				StartRespawn(Controller, RespawnTime);
			}
		}
	}
}

void AGunsmithGameMode::RespawnPlayer(AController* Controller, FTimerHandle Handle)
{
	if (Handle.IsValid())
	{
		PendingRespawnHandles.Remove(Handle);
	}
	
	if (IsValid(Controller))
	{
		if (Controller->GetPawn())
		{
			UE_LOG(LogGunsmith, Error, TEXT("Unable to respawn Controller %s as the pawn still exists. Make sure it has been destroyed before this point."), *Controller->GetName());
		}
					
		RestartPlayer(Controller);
	}
}
