// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/Tests/GunsmithGameState_Accuracy.h"
#include "GunsmithGameState_HitResponse.generated.h"

/**
 * A game state used to test a players hit response when hit by all other players
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithGameState_HitResponse : public AGunsmithGameState_Accuracy
{
	GENERATED_BODY()
	
public:
	AGunsmithGameState_HitResponse();
	
	// AGunsmithGameState_Accuracy Begin
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	virtual void Tick(float DeltaSeconds) override;
	// AGunsmithGameState_Accuracy End
};
