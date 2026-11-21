// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GunsmithBlueprintFunctionLibrary.generated.h"

class UAnimInstance;
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

	// Get the root focus widget at the top of the tree via UWidget->GetDesiredFocusWidget
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI")
	static UWidget* GetRootFocusWidget(UWidget* Widget);

	// Return true if the world is currently playing a replay
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation", meta = (WorldContext="WorldContextObject"))
	static bool IsReplayPlaying(UObject* WorldContextObject);

	/**
	 * Plays a context effect (e.g. a footstep) from an AnimBP named-notify event such as AnimNotify_Footstep_Walk.
	 * The Gunsmith plugin's animations only contain named notifies, so this does what UAnimNotify_LyraContextEffects::Notify did:
	 * optionally traces for the surface, then calls AnimMotionEffect on the owning actor and every component implementing
	 * ILyraContextEffectsInterface (e.g. ULyraContextEffectComponent).
	 *
	 * @param AnimInstance		The AnimBP calling this (filled in automatically)
	 * @param Effect			Effect to play, e.g. AnimEffect.Footstep.Walk
	 * @param SocketName		Socket to attach the effect to and start the trace from. Only used when bAttached is true
	 * @param bAttached			Attach the effect to SocketName. If false it plays at, and traces from, the mesh origin
	 * @param bPerformTrace		Trace for the surface. Required for physical surface to context conversion
	 * @param TraceEndOffset	Trace end, relative to the trace start
	 */
	UFUNCTION(BlueprintCallable, Category = "Effects", meta = (DefaultToSelf = "AnimInstance", HidePin = "AnimInstance", AdvancedDisplay = "LocationOffset,RotationOffset,VFXScale,AudioVolume,AudioPitch,bIgnoreOwner"))
	static void PlayContextEffect(UAnimInstance* AnimInstance, FGameplayTag Effect, FName SocketName = NAME_None,
		bool bAttached = false, bool bPerformTrace = true, FVector TraceEndOffset = FVector(0, 0, -50),
		TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility, bool bIgnoreOwner = true,
		FVector LocationOffset = FVector::ZeroVector, FRotator RotationOffset = FRotator::ZeroRotator,
		FVector VFXScale = FVector(1, 1, 1), float AudioVolume = 1.f, float AudioPitch = 1.f);
};
