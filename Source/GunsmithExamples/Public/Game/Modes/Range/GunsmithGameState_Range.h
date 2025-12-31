// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/GunsmithGameState.h"
#include "GunsmithGameState_Range.generated.h"

class UGSWeaponDataAsset;
class UGSShootingComponent;
struct FGSDamageRecord;
struct FGSEquipData;

DECLARE_MULTICAST_DELEGATE_TwoParams(FGunsmithRangeScoreChangedDelegate, UGSWeaponDataAsset* WeaponData, float Score);
DECLARE_MULTICAST_DELEGATE_TwoParams(FGunsmithRangeTrackedWeaponChangedDelegate, UGSShootingComponent* ShootingComponent, UGSWeaponDataAsset* NewWeaponData);
DECLARE_MULTICAST_DELEGATE_OneParam(FGunsmithRangeActiveStateChangedDelegate, bool bActive);
DECLARE_MULTICAST_DELEGATE_TwoParams(FGunsmithRangeTimeRemainingChangedDelegate, int32 TimeRemaining, bool bWarmUpActive);

/**
 *	A game state to track range scores
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithGameState_Range : public AGunsmithGameState
{
	GENERATED_BODY()

public:
	// AGameState Begin
	AGunsmithGameState_Range();
	virtual void Tick(float DeltaSeconds) override;
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// AGameState End
	
	FGunsmithRangeScoreChangedDelegate OnScoreChanged;
	FGunsmithRangeScoreChangedDelegate OnRecordChanged;
	FGunsmithRangeTrackedWeaponChangedDelegate OnWeaponChanged;
	FGunsmithRangeActiveStateChangedDelegate OnPractiseActiveChanged;
	FGunsmithRangeTimeRemainingChangedDelegate OnPractiseTimeRemainingChanged;

	// Sets the practise state to enabled
	void StartPractise(float WarmUpTime);
	// Ends the current practise state
	void EndPractise();

	UGSWeaponDataAsset* GetTrackedWeapon() const { return CurrentEquippedWeapon.Get(); }
	APawn* GetTrackedPawn() const { return CurrentTrackedPawn.Get(); }
	float GetCurrentScore() const { return CurrentScore; }
	float GetRecordForWeapon(UGSWeaponDataAsset* WeaponData) const;
	bool IsPractiseActive() const { return bIsPractiseActive; }
	
protected:

	UPROPERTY(EditDefaultsOnly, Category = "Practise", meta=(ClampMin="0"))
	float PractiseRoundTime = 30.0f;
	
	float CurrentScore = 0.0f;
	TWeakObjectPtr<UGSWeaponDataAsset> CurrentEquippedWeapon = nullptr;
	TWeakObjectPtr<APawn> CurrentTrackedPawn = nullptr;

	TMap<TObjectKey<UGSWeaponDataAsset>, float> Records;

	UPROPERTY(ReplicatedUsing=OnRep_bIsPractiseActive)
	bool bIsPractiseActive = false;
	float WarmUpTimeRemaining = 0.0f;
	float PractiseTimeRemaining = 0.0f;

	UFUNCTION()
	void OnPawnSet(APlayerState* Player, APawn* NewPawn, APawn* OldPawn);

	UFUNCTION()
	void OnDamageDealt(const FGSDamageRecord& DamageRecord);

	UFUNCTION()
	void OnWeaponEquipped(const FGSEquipData& Weapon);
	
	UFUNCTION()
	void OnRep_bIsPractiseActive();

	// Set the current weapons score value
	void SetCurrentScore(float NewScore);
};
