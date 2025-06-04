// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GunsmithTestPlayerController.generated.h"

/**
 *	A player controller used to start Gunsmith test scenarios
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithTestPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	// Called to change the servers current auto shoot index for accuracy testing
	UFUNCTION(Server, Reliable, Category="Tests")
	void Server_SetAutoShootIndex(int32 Index, FName BoneName);
};
