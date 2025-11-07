// Copyright Kraken Party Limited. All Rights Reserved.


#include "Game/Modes/Range/GunsmithRangeGameMode.h"

#include "AIController.h"
#include "GSGameplayLibrary.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Game/Modes/Range/GunsmithGameState_Range.h"
#include "Game/Modes/Range/GunsmithRangeTargetActor.h"
#include "GameFramework/PlayerState.h"
#include "Weapon/GSShootingComponent.h"
#include "Weapon/Clip/GSClipBehavior.h"
#include "Weapon/Emitter/GSWeaponEmitter.h"
#include "Weapon/Emitter/Output/GSEmitterOutputDataAsset.h"


void AGunsmithRangeGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (TargetActors.Num() > 0 && !StartGameTimer.IsValid())
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

void AGunsmithRangeGameMode::RegisterTargetActor(AGunsmithRangeTargetActor* TargetActor)
{
	TargetActors.Emplace(TargetActor);

	if (TargetActors.Num() > NumActiveTargets && !StartGameTimer.IsValid())
	{
		StartGameTimer = GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			StartGame();	
		});
	}

	TargetActor->OnActiveStateChanged.AddUObject(this, &AGunsmithRangeGameMode::OnTargetActiveStateChanged, TargetActor);
}

void AGunsmithRangeGameMode::ActivateTargets(int32 NumTargets, float TimeBeforeActivation, AGunsmithRangeTargetActor* IgnoredTarget)
{
	if (TimeBeforeActivation > 0.0f && !ActivateTimer.IsValid())
	{
		GetWorld()->GetTimerManager().SetTimer(ActivateTimer, FTimerDelegate::CreateWeakLambda(this, [this, NumTargets, IgnoredTarget]()
		{
			ActivateTargets(NumTargets, 0.0f, IgnoredTarget);	
		}), TimeBeforeActivation, false);
		return;
	}

	ActivateTimer.Invalidate();
	
	for (int32 TargetIndex = 0; TargetIndex < NumTargets; TargetIndex++)
	{
		TArray<TWeakObjectPtr<AGunsmithRangeTargetActor>> ActorsCopy = TargetActors;
		while (ActorsCopy.Num() > 0)
		{
			const int32 RandomTarget = FMath::RandRange(0, ActorsCopy.Num() - 1);
			const TWeakObjectPtr<AGunsmithRangeTargetActor>& Target = ActorsCopy[RandomTarget];
			const float TargetRange = GetTargetDistanceFromRange(Target);
			
			if (Target.IsValid() && !Target->IsActive() && Target != IgnoredTarget && TargetRange <= MaxWeaponRange)
			{
				Target->SetTargetActive(true);
				break;
			}

			ActorsCopy.RemoveAt(RandomTarget);
		}
	}
}

void AGunsmithRangeGameMode::ReactivateTargets(float TimeBeforeSpawn)
{
	bIgnoreInactiveCallback = true;
	
	for (const TWeakObjectPtr<AGunsmithRangeTargetActor>& Target : TargetActors)
	{
		if (Target.IsValid())
		{
			Target->SetTargetActive(false);
		}
	}

	bIgnoreInactiveCallback = false;

	if (ActivateTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(ActivateTimer);
		ActivateTimer.Invalidate();
	}

	ActivateTargets(NumActiveTargets, TimeBeforeSpawn);
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
			
			ReactivateTargets(StartUpTime);
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
	if (bStarted)
	{
		return;
	}
	
	bStarted = true;
	
	ActivateTargets(NumActiveTargets);
}

float AGunsmithRangeGameMode::GetTargetDistanceFromRange(
	const TWeakObjectPtr<AGunsmithRangeTargetActor>& TargetActor) const
{
	if (TargetActor.IsValid())
	{
		const FVector ActorLocation = TargetActor->GetActorLocation();
		return FMath::Abs(RangeYLocation - ActorLocation.Y);
	}

	return 0.0f;
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

	for (const TWeakObjectPtr<AGunsmithRangeTargetActor>& Target : TargetActors)
	{
		if (Target.IsValid())
		{
			const float YDistance = GetTargetDistanceFromRange(Target);

			if (YDistance > MaxWeaponRange)
			{
				Target->SetTargetActive(false);
			}
		}
	}
}

void AGunsmithRangeGameMode::OnTargetActiveStateChanged(bool bActive, AGunsmithRangeTargetActor* TargetActor)
{
	if (!bActive && !bIgnoreInactiveCallback)
	{
		ActivateTargets(1, 0.0f, TargetActor);
	}
}
