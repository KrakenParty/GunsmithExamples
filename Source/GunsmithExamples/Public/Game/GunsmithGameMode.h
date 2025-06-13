// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GunsmithGameMode.generated.h"

/**
 *	A basic game mode that respawns characters after death
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	// AGameModeBase Begin
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;
	// AGameModeBase End

protected:
	// If true, characters will be respawned after death
	UPROPERTY(EditDefaultsOnly, Category = "Mode")
	bool bShouldAutoRespawn = true;
	
	// How long to wait before spawning a character after death
	UPROPERTY(EditDefaultsOnly, Category = "Mode", meta=(EditCondition="bShouldAutoRespawn==true"))
	float RespawnTime = 5.0f;
	
private:
	// Called when a pawn dies to start the respawn timer
	UFUNCTION()
	virtual void OnPawnDeath(UGSHealthComponent* HealthComponent, const FGSDamageRecord& DamageRecord, bool bIsPredicted);
};
