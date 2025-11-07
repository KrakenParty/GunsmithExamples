// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/GunsmithGameMode.h"
#include "Weapon/GSEquipData.h"
#include "GunsmithRangeGameMode.generated.h"

class AGunsmithRangeTargetActor;
class AGunsmithRangeSpawnArea;
class UGSWeaponDataAsset;
class AAIController;
class UGSShootingComponent;

/**
 *	A game mode that spawns targets for target practise
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithRangeGameMode : public AGunsmithGameMode
{
	GENERATED_BODY()
	
public:
	// AGunsmithGameMode Begin
	virtual void BeginPlay() override;
	// AGunsmithGameMode Begin

	// Register a target actor that should've been placed in the level
	void RegisterTargetActor(AGunsmithRangeTargetActor* TargetActor);

	void ActivateTargets(int32 NumTargets, float TimeBeforeActivation = 0.0f, AGunsmithRangeTargetActor* IgnoredTarget = nullptr);
	// Deactivate targets and reactivate again after a set time
	void ReactivateTargets(float TimeBeforeSpawn);

	// Begins the practise mode
	void StartPractise();
	// Ends the practise mode
	void EndPractise();

protected:	
	UPROPERTY()
	TArray<TWeakObjectPtr<AGunsmithRangeTargetActor>> TargetActors;

	// How long to wait before spawning the first pawn and starting the timer
	UPROPERTY(EditDefaultsOnly, Category="Range Mode", meta=(ClampMin="0"))
	float StartUpTime = 1.0f;

	// The Y value of the range area where the player stands
	UPROPERTY(EditDefaultsOnly, Category="Range Mode")
	float RangeYLocation = 0.0f;

	// How many targets should be active to shoot at
	UPROPERTY(EditDefaultsOnly, Category="Range Mode", meta=(ClampMin="0"))
	int32 NumActiveTargets = 3;

	// The attachment to apply to characters during the practise mode
	UPROPERTY(EditDefaultsOnly, Category="Range Mode")
	FGSEquipAttachmentData PractiseModeAttachment;

	float MaxWeaponRange = 99999999.9f;
	float LastSpawnDistance = 0.0f;

	bool bStarted = false;
	bool bIgnoreInactiveCallback = false;
	FTimerHandle StartGameTimer;
	FTimerHandle ActivateTimer;

	TMap<TObjectKey<APawn>, int32> AppliedAttachments;

	// Creates the initial bot in the designated spawn area
	void StartGame();

	float GetTargetDistanceFromRange(const TWeakObjectPtr<AGunsmithRangeTargetActor>& TargetActor) const;

	void OnWeaponChanged(UGSShootingComponent* ShootingComponent, UGSWeaponDataAsset* NewWeaponData);

	void OnTargetActiveStateChanged(bool bActive, AGunsmithRangeTargetActor* TargetActor);
};
