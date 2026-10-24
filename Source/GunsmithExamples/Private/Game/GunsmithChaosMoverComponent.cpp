// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/GunsmithChaosMoverComponent.h"


#include "DefaultMovementSet/CharacterMoverSimulationTypes.h"

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 6
#include "ChaosMover/ChaosMoverLog.h"
#include "ChaosMover/Backends/ChaosMoverBackend.h"
#include "ChaosMover/Character/ChaosCharacterInputs.h"
#include "ChaosMover/Character/Modes/ChaosFallingMode.h"
#include "ChaosMover/Character/Modes/ChaosFlyingMode.h"
#include "ChaosMover/Character/Modes/ChaosWalkingMode.h"
#endif

UGunsmithChaosMoverComponent::UGunsmithChaosMoverComponent()
{
	// Default movement modes
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 6
	MovementModes.Add(DefaultModeNames::Walking, CreateDefaultSubobject<UChaosWalkingMode>(TEXT("DefaultChaosWalkingMode")));
	MovementModes.Add(DefaultModeNames::Falling, CreateDefaultSubobject<UChaosFallingMode>(TEXT("DefaultChaosFallingMode")));
	MovementModes.Add(DefaultModeNames::Flying, CreateDefaultSubobject<UChaosFlyingMode>(TEXT("DefaultChaosFlyingMode")));
#endif

	StartingMovementMode = DefaultModeNames::Falling;

	bHandleJump = false;
	bHandleStanceChanges = false;

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 6
	BackendClass = UChaosMoverBackendComponent::StaticClass();
#endif
}

bool UGunsmithChaosMoverComponent::TryGetFloorCheckHitResult(FHitResult& OutHitResult) const
{
	if (bFloorResultSet)
	{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 6
		OutHitResult = LatestFloorResult.HitResult;
		return true;
#endif
	}
		
	return Super::TryGetFloorCheckHitResult(OutHitResult);
}

void UGunsmithChaosMoverComponent::ProduceInput(const int32 DeltaTimeMS, FMoverInputCmdContext* Cmd)
{
	Super::ProduceInput(DeltaTimeMS, Cmd);

	check(Cmd);

	// Add launch input data if launch velocity is set
	if (!LaunchVelocityOrImpulse.IsZero())
	{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 6
		FChaosMoverLaunchInputs& LaunchInputs = Cmd->InputCollection.FindOrAddMutableDataByType<FChaosMoverLaunchInputs>();
		LaunchInputs.LaunchVelocityOrImpulse = LaunchVelocityOrImpulse;
		LaunchInputs.Mode = LaunchMode;
#endif

		LaunchVelocityOrImpulse = FVector::ZeroVector;
	}

	if (QueuedModeTransitionName != NAME_None)
	{
		FCharacterDefaultInputs& CharacterDefaultInputs = Cmd->InputCollection.FindOrAddMutableDataByType<FCharacterDefaultInputs>();
		CharacterDefaultInputs.SuggestedMovementMode = QueuedModeTransitionName;

		ClearQueuedMode();
	}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 6
	// TODO @Harsha Making bWantsToCrouch sticky because the server is missing the one time crouch input.
	// Investigate the input drops and after the fix, revert this change back to making it non-sticky.
	bWantsToCrouch &= CanCrouch();
	FChaosMoverCrouchInputs& CrouchInputs = Cmd->InputCollection.FindOrAddMutableDataByType<FChaosMoverCrouchInputs>();
	CrouchInputs.bWantsToCrouch = bWantsToCrouch;

	if (bCancelMovementOverrides)
	{
		FChaosMovementSettingsOverridesRemover& MovementSettingsRemover = Cmd->InputCollection.FindOrAddMutableDataByType<FChaosMovementSettingsOverridesRemover>();
		MovementSettingsRemover.ModeName = ModeToOverrideSettings;

		bCancelMovementOverrides = false;
	}

	if (bOverrideMovementSettings)
	{
		FChaosMovementSettingsOverrides& MovementSettings = Cmd->InputCollection.FindOrAddMutableDataByType<FChaosMovementSettingsOverrides>();
		MovementSettings.MaxSpeedOverride = MaxSpeedOverride;
		MovementSettings.AccelerationOverride = AccelerationOverride;
		MovementSettings.ModeName = ModeToOverrideSettings;

		bOverrideMovementSettings = false;
	}
#endif
}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 6
void UGunsmithChaosMoverComponent::DoQueueNextMode(FName DesiredModeName, bool bShouldReenter)
{
	Super::DoQueueNextMode(DesiredModeName, bShouldReenter);

	if (QueuedModeTransitionName != NAME_None)
	{ 
		const FName NextModeName = QueuedModeTransitionName;
		if (NextModeName != NAME_None)
		{
			UE_LOG(LogChaosMover, Log, TEXT("%s (%s) Overwriting of queued mode change (%s) with (%s)"), *GetNameSafe(GetOwner()), *UEnum::GetValueAsString(GetOwner()->GetLocalRole()), *NextModeName.ToString(), *DesiredModeName.ToString());
		}
	}

	QueuedModeTransitionName = DesiredModeName;
}
#endif

void UGunsmithChaosMoverComponent::Crouch()
{
	Super::Crouch();
	bCrouchInputPending = true;
}

void UGunsmithChaosMoverComponent::UnCrouch()
{
	Super::UnCrouch();
	bCrouchInputPending = true;
}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 6
void UGunsmithChaosMoverComponent::Launch(const FVector& VelocityOrImpulse, EChaosMoverVelocityEffectMode Mode)
{
	LaunchVelocityOrImpulse = VelocityOrImpulse;
	LaunchMode = Mode;
}

void UGunsmithChaosMoverComponent::OverrideMovementSettings(const FChaosMovementSettingsOverrides Overrides)
{
	ModeToOverrideSettings = Overrides.ModeName;
	MaxSpeedOverride = FMath::Max(Overrides.MaxSpeedOverride, 0.0f);
	AccelerationOverride = FMath::Max(Overrides.AccelerationOverride, 0.0f);
	bOverrideMovementSettings = true;
}
#endif

void UGunsmithChaosMoverComponent::CancelMovementSettingsOverrides(FName ModeName)
{
	ModeToOverrideSettings = ModeName;
	bCancelMovementOverrides = true;
}

void UGunsmithChaosMoverComponent::OnMoverPreSimulationTick(const FMoverTimeStep& TimeStep,
                                                            const FMoverInputCmdContext& InputCmd)
{
	// This overrides the base class to make sure that it doesn't do anything. We do this
	// because the stance modifier and jump handling of the base is not async physics friendly
}

void UGunsmithChaosMoverComponent::ProcessSimulationEvent(const FMoverSimulationEventData& EventData)
{
	Super::ProcessSimulationEvent(EventData);
	
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 6
	if (const FStanceModifiedEventData* StanceModifiedEvent = EventData.CastTo<FStanceModifiedEventData>())
	{
		OnStanceModified(*StanceModifiedEvent);
	}
#endif
}

void UGunsmithChaosMoverComponent::SetAdditionalSimulationOutput(const FMoverDataCollection& Data)
{
	Super::SetAdditionalSimulationOutput(Data);

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 6
	if (const FFloorResultData* FloorData = Data.FindDataByType<FFloorResultData>())
	{
		bFloorResultSet = true;
		LatestFloorResult = FloorData->FloorResult;
	}
#endif
}

void UGunsmithChaosMoverComponent::ClearQueuedMode()
{
	QueuedModeTransitionName = NAME_None;
}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 6
void UGunsmithChaosMoverComponent::OnStanceModified(const FStanceModifiedEventData& EventData)
{
	OnStanceChanged.Broadcast(EventData.OldStance, EventData.NewStance);
}
#endif