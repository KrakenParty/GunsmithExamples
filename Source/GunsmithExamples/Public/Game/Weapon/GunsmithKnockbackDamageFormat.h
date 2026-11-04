// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/GSCurveValue.h"
#include "Weapon/Damage/GSRadialDamageFormat.h"
#include "GunsmithKnockbackDamageFormat.generated.h"

/**
 *	A radial damage format that also applies knockback to any hit targets
 *	When using Chaos, this also applies force to simulating physics objects, including those which cannot receive damage
 */
UCLASS(DisplayName="Knockback Radial Damage")
class GUNSMITHEXAMPLES_API UGunsmithKnockbackDamageFormat : public UGSRadialDamageFormat
{
	GENERATED_BODY()

public:
	// UGSRadialDamageFormat Begin
	virtual void DamageTarget(const FHitResult& InitialHit, const FVector& DamageLocation, AActor* HitActor, const FGSRadialDamageDealt& DamageInfo, int32 Frame, AActor* Instigator, AController* InstigatingController, const FGSDamageFormatDebugSetup& DebugSetup, float CustomMultiplier, bool bShouldReapplyOnReconcile, EGSDamageApplicationMode DamageApplicationMode) override;
	virtual bool CanDamageTarget_Implementation(AActor* Target, AActor* DamageCauser) override;
	// UGSRadialDamageFormat End

	// UGSDamageFormat Begin
	virtual bool CanHitTarget_Implementation(AActor* Target, AActor* DamageCauser) override;
	// UGSDamageFormat End

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Knockback")
	FGSCurveValue LaunchPower = FGSCurveValue(EAlphaBlendOption::Linear, 100.0f, 1.0f, 0.4f);

	// The lowest possible launch angle used to ensure the target is always moved when being hit
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Knockback")
	float MinLaunchAngle = 20.0f;

private:
	// Returns true if the simulation is running on Chaos
	bool GetUsesChaos() const;

	// Returns true if the target has no health and can only be affected by force
	bool IsForceOnlyTarget(const AActor* Target) const;
	
	// Returns the component to inflict the physics impact on
	static UPrimitiveComponent* FindSimulatingComponent(const AActor* Actor, UPrimitiveComponent* PreferredComponent = nullptr);
};
