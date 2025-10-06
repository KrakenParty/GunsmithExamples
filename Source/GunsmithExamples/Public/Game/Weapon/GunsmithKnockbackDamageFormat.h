// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/GSCurveValue.h"
#include "Weapon/Damage/GSRadialDamageFormat.h"
#include "GunsmithKnockbackDamageFormat.generated.h"

/**
 *	A radial damage format that also applies knockback to any hit targets
 */
UCLASS(DisplayName="Knockback Radial Damage")
class GUNSMITHEXAMPLES_API UGunsmithKnockbackDamageFormat : public UGSRadialDamageFormat
{
	GENERATED_BODY()

public:
	// UGSRadialDamageFormat Begin
	virtual void DamageTarget(const FHitResult& InitialHit, const FVector& DamageLocation, AActor* HitActor, const FGSRadialDamageDealt& DamageInfo, int32 Frame, AActor* Instigator, AController* InstigatingController, const FGSDamageFormatDebugSetup& DebugSetup, float CustomMultiplier, bool bShouldReapplyOnReconcile, EGSDamageApplicationMode DamageApplicationMode) override;
	// UGSRadialDamageFormat End
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Knockback")
	FGSCurveValue LaunchPower = FGSCurveValue(EAlphaBlendOption::Linear, 100.0f, 1.0f, 0.4f);

	// The lowest possible launch angle used to ensure the target is always moved when being hit
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Knockback")
	float MinLaunchAngle = 20.0f;
};
