// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "GunsmithPriorityPlayerStart.generated.h"

/**
 *	A player start that is selected before any other available player starts
 */
UCLASS(Blueprintable, ClassGroup=Common)
class GUNSMITHEXAMPLES_API AGunsmithPriorityPlayerStart : public APlayerStart
{
	GENERATED_BODY()
};
