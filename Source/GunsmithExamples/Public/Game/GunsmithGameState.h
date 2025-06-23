// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GunsmithGameState.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGunsmithTests, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGunsmithPlayerListChangedDelegate, APlayerState*, State);

/**
 *	A simple GameState to accompany the GunsmithGameMode
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithGameState : public AGameState
{
	GENERATED_BODY()

public:
	// AGameState Begin
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	// AGameState End

	UPROPERTY(BlueprintAssignable, Category="Gunsmith")
	FGunsmithPlayerListChangedDelegate OnPlayerAdded;
	UPROPERTY(BlueprintAssignable, Category="Gunsmith")
	FGunsmithPlayerListChangedDelegate OnPlayerRemoved;
};
