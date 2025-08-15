// Copyright Kraken Party Limited. All Rights Reserved.


#include "Game/Modes/Range/GunsmithRangeGameMode.h"

#include "AIController.h"
#include "GSGameplayLibrary.h"
#include "GunsmithMoverCharacter.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Game/Modes/Range/GunsmithGameState_Range.h"
#include "Game/Modes/Range/GunsmithRangeSpawnArea.h"
#include "GameFramework/PlayerState.h"
#include "Weapon/GSShootingComponent.h"
#include "Weapon/Clip/GSClipBehavior.h"
#include "Weapon/Emitter/GSWeaponEmitter.h"
#include "Weapon/Emitter/Output/GSEmitterOutputDataAsset.h"


void AGunsmithRangeGameMode::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	AIController = World->SpawnActor<AAIController>();

	if (SpawnArea.IsValid())
	{
		StartGame();
	}

	if (AGunsmithGameState_Range* RangeState = GetWorld()->GetGameState<AGunsmithGameState_Range>())
	{
		RangeState->OnWeaponChanged.AddUObject(this, &AGunsmithRangeGameMode::OnWeaponChanged);

		UGSShootingComponent* ShootingComponent = UGSGameplayLibrary::GetShootingComponentFromActor(RangeState->GetTrackedPawn());
		OnWeaponChanged(ShootingComponent, RangeState->GetTrackedWeapon());
	}
}

APawn* AGunsmithRangeGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	// Override spawn location for AI characters
	if (NewPlayer->IsA(AAIController::StaticClass()))
	{
		FVector SpawnLocation = FVector::ZeroVector;
		FRotator SpawnRotation = FRotator::ZeroRotator;

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (SpawnArea.IsValid())
		{
			SpawnLocation = SpawnArea->GetRandomSpawnLocation(MaxWeaponRange - SpawnWallThickness, LastSpawnDistance);
			SpawnRotation = SpawnArea->GetActorRotation().GetInverse();
		}

		const FTransform SpawnTransform = FTransform(SpawnRotation, SpawnLocation);
		return SpawnDefaultPawnAtTransform(NewPlayer, SpawnTransform);
	}
	
	return Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
}

AActor* AGunsmithRangeGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	if (Player->IsA(AAIController::StaticClass()) && SpawnArea.IsValid())
	{
		return SpawnArea.Get();
	}
	
	return Super::FindPlayerStart_Implementation(Player, IncomingName);
}

void AGunsmithRangeGameMode::RegisterSpawnArea(AGunsmithRangeSpawnArea* NewArea)
{
	SpawnArea = NewArea;

	if (AIController)
	{
		StartGame();
	}
}

void AGunsmithRangeGameMode::RestartAIPawns(float TimeBeforeSpawn)
{
	// Destroy any active pawns
	if (AIController)
	{
		if (AGunsmithMoverCharacter* Pawn = AIController->GetPawn<AGunsmithMoverCharacter>())
		{
			Pawn->ForceDeath();
		}
	}
	
	// Clear any existing respawns
	for (FTimerHandle& Handle : PendingRespawnHandles)
	{
		GetWorld()->GetTimerManager().ClearTimer(Handle);
	}

	PendingRespawnHandles.Reset();

	// Start a new respawn
	if (AIController)
	{
		StartRespawn(AIController, TimeBeforeSpawn);
	}
}

void AGunsmithRangeGameMode::StartPractise()
{
	if (AGunsmithGameState_Range* RangeState = GetWorld()->GetGameState<AGunsmithGameState_Range>())
	{
		if (!RangeState->IsPractiseActive())
		{
			RangeState->StartPractise(StartUpTime);

			for (APlayerState* Player : RangeState->PlayerArray)
			{
				APawn* Pawn = Player->GetPawn();

				if (IsValid(Pawn))
				{
					if (UGSShootingComponent* ShootingComponent = UGSGameplayLibrary::GetShootingComponentFromActor(Pawn))
					{
						const int32 AttachmentID = ShootingComponent->AddAttachment(PractiseModeAttachment);
						AppliedAttachments.FindOrAdd(Pawn) = AttachmentID;

						// Reload the current ammo
						if (UGSClipBehavior* ClipBehavior = ShootingComponent->GetClipBehavior())
						{
							ClipBehavior->FullReload(ShootingComponent, 0);
						}
					}
				}
			}
			
			RestartAIPawns(StartUpTime);
		}
	}
}

void AGunsmithRangeGameMode::EndPractise()
{
	if (AGunsmithGameState_Range* RangeState = GetWorld()->GetGameState<AGunsmithGameState_Range>())
	{
		RangeState->EndPractise();

		// Remove any applied attachments
		for (APlayerState* Player : RangeState->PlayerArray)
		{
			APawn* Pawn = Player->GetPawn();

			if (IsValid(Pawn) && AppliedAttachments.Contains(Pawn))
			{
				if (UGSShootingComponent* ShootingComponent = UGSGameplayLibrary::GetShootingComponentFromActor(Pawn))
				{
					ShootingComponent->RemoveAttachment(AppliedAttachments[Pawn]);
					AppliedAttachments.Remove(Pawn);
				}
			}
		}
	}
}

void AGunsmithRangeGameMode::StartGame()
{
	RestartPlayer(AIController);
}

void AGunsmithRangeGameMode::OnWeaponChanged(UGSShootingComponent* ShootingComponent, UGSWeaponDataAsset* NewWeaponData)
{
	if (NewWeaponData)
	{
		float MaxRange = TNumericLimits<float>::Max();

		ShootingComponent->ForEachEmitter([&MaxRange](const UGSWeaponEmitter* Emitter)
		{
			if (UGSEmitterOutputDataAsset* DataAsset = Emitter->GetAssociatedEmitterDataAsset())
			{
				MaxRange = FMath::Min(DataAsset->GetMaxRange(), MaxRange);
			}
		});

		MaxWeaponRange = MaxRange;
	}

	if (MaxWeaponRange - SpawnWallThickness < LastSpawnDistance)
	{
		RestartAIPawns(0.0f);
	}
}
