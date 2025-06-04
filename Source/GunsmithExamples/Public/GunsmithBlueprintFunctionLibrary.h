// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GunsmithBlueprintFunctionLibrary.generated.h"

class UMoverComponent;
class UGSMoverComponent;
class ULocalPlayer;

/**
 *	A collection of functions required to create the Gunsmith Samples experience
 */
UCLASS()
class GUNSMITHEXAMPLES_API UGunsmithBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Gets the local player from the Player Controller if valid as there isn't any other function exposed to BP
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player")
	static ULocalPlayer* GetLocalPlayerFromPlayerController(APlayerController* PlayerController);

	// Returns how long the NPP simulation has been running for
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation", meta = (BlueprintThreadSafe, WorldContext="WorldContextObject"))
	static float GetTimeSinceSimulationStarted(UObject* WorldContextObject);

	// Returns a value between 0 and Time base on the TimeSinceSimulationStarted value
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation", meta = (BlueprintThreadSafe, WorldContext="WorldContextObject"))
	static float GetRemainderBaseOnTimeSinceSimulationStarted(UObject* WorldContextObject, float Time);
};
