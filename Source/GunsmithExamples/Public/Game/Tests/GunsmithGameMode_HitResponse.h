// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/GunsmithGameMode.h"
#include "GunsmithGameMode_HitResponse.generated.h"

/**
 * 
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithGameMode_HitResponse : public AGunsmithGameMode
{
	GENERATED_BODY()
	
public:
	// AGunsmithGameMode Begin
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;
	// AGunsmithGameMode End
};
