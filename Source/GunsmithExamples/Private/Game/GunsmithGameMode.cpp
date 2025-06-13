// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/GunsmithGameMode.h"

#include "GSGameplayLibrary.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Health/GSHealthComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogGSGameMode, Log, All);

APawn* AGunsmithGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	APawn* Pawn =  Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
	
	if (UGSHealthComponent* HealthComponent = UGSGameplayLibrary::GetHealthComponentFromActor(Pawn))
	{
		HealthComponent->OnDeath.AddUniqueDynamic(this, &AGunsmithGameMode::OnPawnDeath);
	}
	
	return Pawn;
}

void AGunsmithGameMode::OnPawnDeath(UGSHealthComponent* HealthComponent, const FGSDamageRecord& DamageRecord, bool bIsPredicted)
{
	if (bShouldAutoRespawn)
	{
		if (APawn* OwningPawn = Cast<APawn>(HealthComponent->GetOwner()))
		{
			if (AController* Controller = OwningPawn->GetController())
			{
				// Respawn after RespawnTime seconds
				FTimerHandle TimerHandle;
				GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateWeakLambda(this, [this, Controller]()
				{
					if (IsValid(Controller))
					{
						if (Controller->GetPawn())
						{
							UE_LOG(LogGSGameMode, Error, TEXT("Unable to respawn Controller %s as the pawn still exists. Make sure it has been destroyed before this point."), *Controller->GetName());
						}
					
						RestartPlayer(Controller);
					}
				}), RespawnTime, false);
			}
		}
	}
}
