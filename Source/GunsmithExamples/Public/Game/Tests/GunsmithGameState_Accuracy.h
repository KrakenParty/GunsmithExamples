// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/GunsmithGameState.h"
#include "Weapon/GSShootingComponentSimulation.h"
#include "GunsmithGameState_Accuracy.generated.h"

/**
 *	A game state used to test shooting accuracy
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithGameState_Accuracy : public AGunsmithGameState
{
	GENERATED_BODY()

public:
	// AGunsmithGameState Begin
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// AGunsmithGameState End

	// Sets the current auto shoot index. Should only be called by the server and repped down to clients
	void SetAutoShootData(int32 Index, FName BoneName);
	
protected:
	UPROPERTY(ReplicatedUsing=OnRep_AutoShootData)
	FGunsmithAutoShootData AutoShootData;

private:
	UFUNCTION()
	void OnRep_AutoShootData();
};