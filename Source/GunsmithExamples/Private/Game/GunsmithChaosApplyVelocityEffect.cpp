// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/GunsmithChaosApplyVelocityEffect.h"

#include "MoverSimulationTypes.h"
#include "Runtime/Launch/Resources/Version.h"

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 6
#include "ChaosMover/Character/Effects/ChaosCharacterApplyVelocityEffect.h"
#endif

bool FGunsmithChaosApplyVelocityEffect::ApplyMovementEffect_Async(FApplyMovementEffectParams_Async& ApplyEffectParams, FMoverSyncState& OutputState)
{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 6
	// Reuse the Chaos effect for the velocity change so it stays in sync with engine behaviour
	FChaosCharacterApplyVelocityEffect ChaosVelocityEffect;
	ChaosVelocityEffect.VelocityOrImpulseToApply = VelocityToApply;
	ChaosVelocityEffect.Mode = bAdditiveVelocity ? EChaosMoverVelocityEffectMode::AdditiveVelocity : EChaosMoverVelocityEffectMode::OverrideVelocity;

	if (!ChaosVelocityEffect.ApplyMovementEffect_Async(ApplyEffectParams, OutputState))
	{
		return false;
	}

	// The Chaos state machine switches mode immediately when an instant effect changes the output mode
	// Only set it when provided, as an empty name would put the simulation into no mode
	if (!ForceMovementMode.IsNone())
	{
		OutputState.MovementMode = ForceMovementMode;
	}

	return true;
#else
	return false;
#endif
}

FInstantMovementEffect* FGunsmithChaosApplyVelocityEffect::Clone() const
{
	FGunsmithChaosApplyVelocityEffect* CopyPtr = new FGunsmithChaosApplyVelocityEffect(*this);
	return CopyPtr;
}

UScriptStruct* FGunsmithChaosApplyVelocityEffect::GetScriptStruct() const
{
	return FGunsmithChaosApplyVelocityEffect::StaticStruct();
}

FString FGunsmithChaosApplyVelocityEffect::ToSimpleString() const
{
	return FString::Printf(TEXT("GunsmithChaosApplyVelocity VelocityToApply = %s, bAdditiveVelocity = %d, ForceMovementMode = %s"), *VelocityToApply.ToString(), bAdditiveVelocity, *ForceMovementMode.ToString());
}
