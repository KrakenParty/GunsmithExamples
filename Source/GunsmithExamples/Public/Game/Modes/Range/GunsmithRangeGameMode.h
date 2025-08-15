// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/GunsmithGameMode.h"
#include "Weapon/GSEquipData.h"
#include "GunsmithRangeGameMode.generated.h"

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
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;
	virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName = TEXT("")) override;
	// AGunsmithGameMode Begin

	// Register a spawn area on the map for bots to be spawned in
	void RegisterSpawnArea(AGunsmithRangeSpawnArea* NewArea);

	// Destroy all existing pawns and starts again after a set time
	void RestartAIPawns(float TimeBeforeSpawn);

	// Begins the practise mode
	void StartPractise();
	// Ends the practise mode
	void EndPractise();

protected:	
	UPROPERTY()
	TObjectPtr<AAIController> AIController = nullptr;

	UPROPERTY()
	TWeakObjectPtr<AGunsmithRangeSpawnArea> SpawnArea = nullptr;

	// How long to wait before spawning the first pawn and starting the timer
	UPROPERTY(EditDefaultsOnly, Category="Range Mode", meta=(ClampMin="0"))
	float StartUpTime = 1.0f;

	// How much distance is there between the spawn start and the character
	UPROPERTY(EditDefaultsOnly, Category="Range Mode", meta=(ClampMin="0"))
	float SpawnWallThickness = 100.0f;

	// The attachment to apply to characters during the practise mode
	UPROPERTY(EditDefaultsOnly, Category="Range Mode", meta=(ClampMin="0"))
	FGSEquipAttachmentData PractiseModeAttachment;

	float MaxWeaponRange = 99999999.9f;
	float LastSpawnDistance = 0.0f;

	TMap<TObjectKey<APawn>, int32> AppliedAttachments;

	// Creates the initial bot in the designated spawn area
	void StartGame();

	void OnWeaponChanged(UGSShootingComponent* ShootingComponent, UGSWeaponDataAsset* NewWeaponData);
};
