// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/Modes/Range/GunsmithGameState_Range.h"

#include "GSGameplayLibrary.h"
#include "GameFramework/PlayerState.h"
#include "Health/GSHealthComponent.h"
#include "Weapon/GSShootingComponent.h"

AGunsmithGameState_Range::AGunsmithGameState_Range()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGunsmithGameState_Range::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsPractiseActive)
	{
		if (WarmUpTimeRemaining > 0.0f)
		{
			WarmUpTimeRemaining = FMath::Max(WarmUpTimeRemaining - DeltaSeconds, 0.0f);

			if (WarmUpTimeRemaining == 0.0f)
			{
				OnPractiseTimeRemainingChanged.Broadcast(FMath::CeilToInt(PractiseTimeRemaining), false);
			}
		}
		else
		{
			const float NewTime = FMath::Max(PractiseTimeRemaining - DeltaSeconds, 0.0f);
			const int32 NewTimeCeil = FMath::CeilToInt(NewTime);
			bool bHasTimeChanged = NewTimeCeil != FMath::CeilToInt(PractiseTimeRemaining);

			PractiseTimeRemaining = NewTime;

			if (bHasTimeChanged)
			{
				if (PractiseTimeRemaining == 0.0f)
				{
					EndPractise();
				}
				else
				{
					OnPractiseTimeRemainingChanged.Broadcast(NewTimeCeil, false);
				}
			}	
		}
	}
}

void AGunsmithGameState_Range::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
	
	PlayerState->OnPawnSet.AddDynamic(this, &AGunsmithGameState_Range::OnPawnSet);

	if (APawn* Pawn = PlayerState->GetPawn())
	{
		OnPawnSet(PlayerState, Pawn, nullptr);
	}
}

void AGunsmithGameState_Range::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	PlayerState->OnPawnSet.RemoveDynamic(this, &AGunsmithGameState_Range::OnPawnSet);
}

void AGunsmithGameState_Range::StartPractise(float WarmUpTime)
{
	if (bIsPractiseActive)
	{
		return;
	}

	bIsPractiseActive = true;

	SetCurrentScore(0.0f);

	WarmUpTimeRemaining = WarmUpTime;
	PractiseTimeRemaining = PractiseRoundTime;

	OnPractiseActiveChanged.Broadcast(true);
	OnPractiseTimeRemainingChanged.Broadcast(FMath::CeilToInt(PractiseTimeRemaining), WarmUpTime > 0.0f);
}

void AGunsmithGameState_Range::EndPractise()
{
	if (!bIsPractiseActive)
	{
		return;
	}

	bIsPractiseActive = false;

	// Commit record score
	if (UGSWeaponDataAsset* WeaponData = CurrentEquippedWeapon.Get())
	{
		float& CurrentRecord = Records.FindOrAdd(WeaponData);

		if (CurrentScore > CurrentRecord)
		{
			CurrentRecord = CurrentScore;
			OnRecordChanged.Broadcast(WeaponData, CurrentScore);
		}
	}

	SetCurrentScore(0.0f);

	OnPractiseActiveChanged.Broadcast(false);
}

float AGunsmithGameState_Range::GetRecordForWeapon(UGSWeaponDataAsset* WeaponData) const
{
	if (Records.Contains(WeaponData))
	{
		return Records[WeaponData];
	}

	return 0.0f;
}

void AGunsmithGameState_Range::OnPawnSet(APlayerState* Player, APawn* NewPawn, APawn* OldPawn)
{
	if (UGSShootingComponent* ShootingComponent = UGSGameplayLibrary::GetShootingComponentFromActor(OldPawn))
	{
		ShootingComponent->OnDamageDealt.RemoveDynamic(this, &AGunsmithGameState_Range::OnDamageDealt);
		ShootingComponent->OnEquipped.RemoveDynamic(this, &AGunsmithGameState_Range::OnWeaponEquipped);

		CurrentTrackedPawn.Reset();
		CurrentEquippedWeapon.Reset();
	}
	
	if (UGSShootingComponent* ShootingComponent = UGSGameplayLibrary::GetShootingComponentFromActor(NewPawn))
	{
		CurrentTrackedPawn = NewPawn;
		
		ShootingComponent->OnDamageDealt.AddDynamic(this, &AGunsmithGameState_Range::OnDamageDealt);
		ShootingComponent->OnEquipped.AddDynamic(this, &AGunsmithGameState_Range::OnWeaponEquipped);
		
		OnWeaponEquipped(ShootingComponent->GetEquippedWeapon());
	}

	SetCurrentScore(0.0f);
}

void AGunsmithGameState_Range::OnDamageDealt(const FGSDamageRecord& DamageRecord)
{
	if (bIsPractiseActive && DamageRecord.bKilledTarget)
	{
		SetCurrentScore(CurrentScore + 1);
	}
}

void AGunsmithGameState_Range::OnWeaponEquipped(const FGSEquipData& Weapon)
{
	if (bIsPractiseActive)
	{
		EndPractise();
	}
	
	CurrentEquippedWeapon = Weapon.WeaponData;

	UGSShootingComponent* ShootingComponent = UGSGameplayLibrary::GetShootingComponentFromActor(CurrentTrackedPawn.Get());
	OnWeaponChanged.Broadcast(ShootingComponent, CurrentEquippedWeapon.Get());
}

void AGunsmithGameState_Range::SetCurrentScore(float NewScore)
{
	CurrentScore = NewScore;

	if (UGSWeaponDataAsset* WeaponData = CurrentEquippedWeapon.Get())
	{
		OnScoreChanged.Broadcast(WeaponData, CurrentScore);
	}
}
