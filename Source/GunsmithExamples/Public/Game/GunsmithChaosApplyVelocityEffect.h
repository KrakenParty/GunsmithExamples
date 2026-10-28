// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DefaultMovementSet/InstantMovementEffects/BasicInstantMovementEffects.h"
#include "GunsmithChaosApplyVelocityEffect.generated.h"

/**
 *	Apply velocity effect that supports the async Chaos Mover simulation
 *	Matches FApplyVelocityEffect by also forcing the movement mode, which FChaosCharacterApplyVelocityEffect does not do
 *	Derives from FApplyVelocityEffect rather than the Chaos effect so the type still exists for UHT in engine versions without Chaos Mover
 */
USTRUCT(BlueprintType, Category = "Gunsmith|Instant Movement Effect", DisplayName = "Gunsmith Chaos Apply Velocity Instant Movement Effect")
struct GUNSMITHEXAMPLES_API FGunsmithChaosApplyVelocityEffect : public FApplyVelocityEffect
{
	GENERATED_BODY()

	virtual bool ApplyMovementEffect_Async(FApplyMovementEffectParams_Async& ApplyEffectParams, FMoverSyncState& OutputState) override;

	virtual FInstantMovementEffect* Clone() const override;

	virtual UScriptStruct* GetScriptStruct() const override;

	virtual FString ToSimpleString() const override;
};
