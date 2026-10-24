// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/GSMoverComponent.h"

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 6
#include "ChaosMover/ChaosMoverSimulationTypes.h"
#include "ChaosMover/Character/ChaosCharacterMoverComponent.h"
#endif

#include "GunsmithChaosMoverComponent.generated.h"

/**
 *	Reimplemented UChaosCharacterMoverComponent logic on top of custom logic in UGSMoverComponent
 */
UCLASS()
class GUNSMITHEXAMPLES_API UGunsmithChaosMoverComponent : public UGSMoverComponent
{
	GENERATED_BODY()
	
public:
	UGunsmithChaosMoverComponent();
	
	virtual bool TryGetFloorCheckHitResult(FHitResult& OutHitResult) const override;
	virtual void ProduceInput(const int32 DeltaTimeMS, FMoverInputCmdContext* Cmd) override;
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 6
	virtual void DoQueueNextMode(FName DesiredModeName, bool bShouldReenter=false) override;
#endif

	virtual void Crouch() override;
	virtual void UnCrouch() override;

	// Launch the character using either impulse or velocity
	// Note: This will only trigger a launch if a launch transition is implemented on the current movement mode
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 6
	void Launch(const FVector& VelocityOrImpulse, EChaosMoverVelocityEffectMode Mode = EChaosMoverVelocityEffectMode::AdditiveVelocity);
#endif

	// Override the movement mode settings on a mode
	// If the name is not set it will apply to the current mode
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 6
	void OverrideMovementSettings(const FChaosMovementSettingsOverrides Overrides);
#endif

	// Cancel overrides of movement mode settings
	// If the name is not set it will apply to the current mode
	UFUNCTION(BlueprintCallable, Category = "Chaos Mover")
	void CancelMovementSettingsOverrides(FName ModeName = NAME_None);
	
protected:
	virtual void OnMoverPreSimulationTick(const FMoverTimeStep& TimeStep, const FMoverInputCmdContext& InputCmd) override;
	virtual void ProcessSimulationEvent(const FMoverSimulationEventData& EventData) override;
	virtual void SetAdditionalSimulationOutput(const FMoverDataCollection& Data) override;

	void ClearQueuedMode();
	
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 6
	virtual void OnStanceModified(const FStanceModifiedEventData& EventData);
#endif
	
	FName ModeToOverrideSettings = NAME_None;
	float MaxSpeedOverride = 0.0f;
	float AccelerationOverride = 0.0f;
	bool bOverrideMovementSettings = false;
	bool bCancelMovementOverrides = false;

	bool bFloorResultSet = false;

	FVector LaunchVelocityOrImpulse = FVector::ZeroVector;
	
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 6
	EChaosMoverVelocityEffectMode LaunchMode = EChaosMoverVelocityEffectMode::AdditiveVelocity;
	FFloorCheckResult LatestFloorResult;
#endif

	// Queued immediate mode transition, it will be transmitted to the PT next time ProduceInput is called
	FName QueuedModeTransitionName = NAME_None;

	bool bCrouchInputPending = false;
};
