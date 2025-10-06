// Copyright Kraken Party Limited. All Rights Reserved.


#include "Game/Weapon/GunsmithKnockbackDamageFormat.h"

#include "DrawDebugHelpers.h"
#include "GunsmithMoverCharacter.h"
#include "Character/GSMoverComponent.h"
#include "DefaultMovementSet/InstantMovementEffects/BasicInstantMovementEffects.h"


void UGunsmithKnockbackDamageFormat::DamageTarget(const FHitResult& InitialHit, const FVector& DamageLocation, AActor* HitActor,
                                                  const FGSRadialDamageDealt& DamageInfo, int32 Frame, AActor* Instigator, AController* InstigatingController,
                                                  const FGSDamageFormatDebugSetup& DebugSetup, float CustomMultiplier, bool bShouldReapplyOnReconcile,
                                                  EGSDamageApplicationMode DamageApplicationMode)
{
	Super::DamageTarget(InitialHit, DamageLocation, HitActor, DamageInfo, Frame, Instigator, InstigatingController, DebugSetup,
	                    CustomMultiplier, bShouldReapplyOnReconcile, DamageApplicationMode);

	AGunsmithMoverCharacter* MoverCharacter = Cast<AGunsmithMoverCharacter>(HitActor);

	if (!MoverCharacter)
	{
		return;
	}

	UGSMoverComponent* MoverComponent = MoverCharacter->GetMoverComponent();

	if (!MoverComponent)
	{
		return;
	}

	FVector HitVector = DamageInfo.HitResult.Location - DamageLocation;
	const FVector HitVectorNormalized = HitVector.GetSafeNormal();
	const float HitRadius = OuterDamageRadius.GetModifiedValue();
	const float HitStrength = HitVector.Size() / FMath::Max(HitRadius, 0.01f);
	const float LaunchStrength = LaunchPower.GetValue(1.0f - HitStrength);

	const FVector TargetUpVector = MoverCharacter->GetActorUpVector();
	const FQuat RotationBetween = FQuat::FindBetween(TargetUpVector, HitVectorNormalized);
	const float AngleBetween = RotationBetween.GetAngle();

	// Ensure the hit is away from the ground
	const float DesiredDegrees = 90.0f - MinLaunchAngle;
	const float DesiredLaunchAngle = FMath::DegreesToRadians(DesiredDegrees);
	if (MinLaunchAngle != 0.0f && AngleBetween > DesiredLaunchAngle)
	{
		HitVector = FMath::VInterpNormalRotationTo(TargetUpVector, HitVectorNormalized, 1.0f, DesiredDegrees);
	}

	// Create a new impulse to be added to the Mover component via Layered Move
	TSharedPtr<FApplyVelocityEffect> LaunchMove = MakeShared<FApplyVelocityEffect>();
	LaunchMove->VelocityToApply = HitVector.GetSafeNormal() * LaunchStrength;
	LaunchMove->bAdditiveVelocity = true;
	LaunchMove->ForceMovementMode = DefaultModeNames::Falling;
	
	MoverComponent->QueueInstantMovementEffect(LaunchMove);
}
