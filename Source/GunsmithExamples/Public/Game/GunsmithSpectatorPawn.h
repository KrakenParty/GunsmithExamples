// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "GunsmithSpectatorPawn.generated.h"

/**
 *	A spectator pawn with the default inputs removed
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithSpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()

public:
	AGunsmithSpectatorPawn();
};
