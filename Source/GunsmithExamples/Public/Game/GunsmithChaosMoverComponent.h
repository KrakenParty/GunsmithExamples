// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ChaosMover/ChaosMoverSimulationTypes.h"
#include "ChaosMover/Character/ChaosCharacterMoverComponent.h"
#include "Character/GSMoverComponent.h"
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
	virtual void DoQueueNextMode(FName DesiredModeName, bool bShouldReenter=false) override;

	virtual void Crouch() override;
	virtual void UnCrouch() override;
	
	// Broadcast when this actor lands on a valid surface.
	UPROPERTY(BlueprintAssignable, Category = "Chaos Mover")
	FChaosMover_OnLanded OnLanded;

	// Broadcast when this actor jumps.
	UPROPERTY(BlueprintAssignable, Category = "Chaos Mover")
	FChaosMover_OnJumped OnJumped;

	// Launch the character using either impulse or velocity
	// Note: This will only trigger a launch if a launch transition is implemented on the current movement mode
	UFUNCTION(BlueprintCallable, Category = "Chaos Mover")
	void Launch(const FVector& VelocityOrImpulse, EChaosMoverVelocityEffectMode Mode = EChaosMoverVelocityEffectMode::AdditiveVelocity);

	// Override the movement mode settings on a mode
	// If the name is not set it will apply to the current mode
	UFUNCTION(BlueprintCallable, Category = "Chaos Mover")
	void OverrideMovementSettings(const FChaosMovementSettingsOverrides Overrides);

	// Cancel overrides of movement mode settings
	// If the name is not set it will apply to the current mode
	UFUNCTION(BlueprintCallable, Category = "Chaos Mover")
	void CancelMovementSettingsOverrides(FName ModeName = NAME_None);
	
protected:
	virtual void OnMoverPreSimulationTick(const FMoverTimeStep& TimeStep, const FMoverInputCmdContext& InputCmd) override;
	virtual void ProcessSimulationEvent(const FMoverSimulationEventData& EventData) override;
	virtual void SetAdditionalSimulationOutput(const FMoverDataCollection& Data) override;

	void ClearQueuedMode();
	virtual void OnStanceModified(const FStanceModifiedEventData& EventData);
	
	FName ModeToOverrideSettings = NAME_None;
	float MaxSpeedOverride = 0.0f;
	float AccelerationOverride = 0.0f;
	bool bOverrideMovementSettings = false;
	bool bCancelMovementOverrides = false;

	bool bFloorResultSet = false;
	FFloorCheckResult LatestFloorResult;

	FVector LaunchVelocityOrImpulse = FVector::ZeroVector;
	EChaosMoverVelocityEffectMode LaunchMode = EChaosMoverVelocityEffectMode::AdditiveVelocity;

	// Queued immediate mode transition, it will be transmitted to the PT next time ProduceInput is called
	FName QueuedModeTransitionName = NAME_None;

	bool bCrouchInputPending = false;
};
