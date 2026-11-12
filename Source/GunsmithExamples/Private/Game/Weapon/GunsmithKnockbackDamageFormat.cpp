// Copyright Kraken Party Limited. All Rights Reserved.


#include "Game/Weapon/GunsmithKnockbackDamageFormat.h"

#include "DrawDebugHelpers.h"
#include "GSGameplayLibrary.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "GunsmithMoverCharacter.h"
#include "Character/GSMoverComponent.h"
#include "DefaultMovementSet/InstantMovementEffects/BasicInstantMovementEffects.h"
#include "World/GSWorldStateSubsystem.h"
#include "Game/GunsmithChaosApplyVelocityEffect.h"
#include "GSChaosGameplayLibrary.h"

void UGunsmithKnockbackDamageFormat::DamageTarget(const FHitResult& InitialHit, const FVector& DamageLocation, AActor* HitActor,
                                                  const FGSRadialDamageDealt& DamageInfo, int32 Frame, AActor* Instigator, AController* InstigatingController,
                                                  const FGSDamageFormatDebugSetup& DebugSetup, float CustomMultiplier, bool bShouldReapplyOnReconcile,
                                                  EGSDamageApplicationMode DamageApplicationMode)
{
	Super::DamageTarget(InitialHit, DamageLocation, HitActor, DamageInfo, Frame, Instigator, InstigatingController, DebugSetup,
	                    CustomMultiplier, bShouldReapplyOnReconcile, DamageApplicationMode);

	const bool bUsesChaos = GetUsesChaos();
	AGunsmithMoverCharacter* MoverCharacter = Cast<AGunsmithMoverCharacter>(HitActor);
	UGSMoverComponent* MoverComponent = MoverCharacter ? MoverCharacter->GetMoverComponent() : nullptr;

	// Characters are launched through their movement component, anything else needs to be simulating to receive a force
	UPrimitiveComponent* PhysicsComponent = nullptr;
	if (!MoverCharacter && bUsesChaos && IsValid(HitActor))
	{
		PhysicsComponent = FindSimulatingComponent(HitActor, DamageInfo.HitResult.GetComponent());
	}

	if (!MoverComponent && !PhysicsComponent)
	{
		return;
	}

	FVector HitVector = DamageInfo.HitResult.Location - DamageLocation;
	const FVector HitVectorNormalized = HitVector.GetSafeNormal();
	const float HitRadius = OuterDamageRadius.GetModifiedValue();
	const float HitStrength = HitVector.Size() / FMath::Max(HitRadius, 0.01f);
	const float LaunchStrength = LaunchPower.GetValue(1.0f - HitStrength);

	// Physics objects can be at any rotation so their up vector can't be used to find the ground
	const FVector TargetUpVector = MoverCharacter ? MoverCharacter->GetActorUpVector() : FVector::UpVector;
	const FQuat RotationBetween = FQuat::FindBetween(TargetUpVector, HitVectorNormalized);
	const float AngleBetween = RotationBetween.GetAngle();

	// Ensure the hit is away from the ground
	const float DesiredDegrees = 90.0f - MinLaunchAngle;
	const float DesiredLaunchAngle = FMath::DegreesToRadians(DesiredDegrees);
	if (MinLaunchAngle != 0.0f && AngleBetween > DesiredLaunchAngle)
	{
		HitVector = FMath::VInterpNormalRotationTo(TargetUpVector, HitVectorNormalized, 1.0f, DesiredDegrees);
	}
	
	if (PhysicsComponent)
	{
		UGSChaosGameplayLibrary::AddImpulse_PhysicsThread(PhysicsComponent, HitVector.GetSafeNormal() * LaunchStrength, NAME_None, true);

		return;
	}

	AsyncTask(ENamedThreads::GameThread, [bUsesChaos, HitVector, LaunchStrength, MoverComponent]()
	{
		if (IsValid(MoverComponent))
		{
			if (bUsesChaos)
			{
				TSharedPtr<FGunsmithChaosApplyVelocityEffect> ChaosVelocityEffect = MakeShared<FGunsmithChaosApplyVelocityEffect>();
				ChaosVelocityEffect->VelocityToApply = HitVector.GetSafeNormal() * LaunchStrength;
				ChaosVelocityEffect->bAdditiveVelocity = true;
				ChaosVelocityEffect->ForceMovementMode = DefaultModeNames::Falling;

				MoverComponent->QueueInstantMovementEffect(ChaosVelocityEffect);
			}
			else
			{
				// Create a new impulse to be added to the Mover component via Layered Move
				TSharedPtr<FApplyVelocityEffect> LaunchMove = MakeShared<FApplyVelocityEffect>();
				LaunchMove->VelocityToApply = HitVector.GetSafeNormal() * LaunchStrength;
				LaunchMove->bAdditiveVelocity = true;
				LaunchMove->ForceMovementMode = DefaultModeNames::Falling;

				MoverComponent->QueueInstantMovementEffect(LaunchMove);
			}
		}
	});
}

bool UGunsmithKnockbackDamageFormat::CanDamageTarget_Implementation(AActor* Target, AActor* DamageCauser)
{
	// Force only targets can be hit, but have no health to damage
	return !IsForceOnlyTarget(Target) && Super::CanDamageTarget_Implementation(Target, DamageCauser);
}

bool UGunsmithKnockbackDamageFormat::CanHitTarget_Implementation(AActor* Target, AActor* DamageCauser)
{
	if (!IsForceOnlyTarget(Target))
	{
		return Super::CanHitTarget_Implementation(Target, DamageCauser);
	}

	if (!bCanHitSelf && DamageCauser == Target)
	{
		return false;
	}

	bool bCanHitTarget = true;

	if (TargetCheckDelegate.IsBound())
	{
		TargetCheckDelegate.Execute(bCanHitTarget, Target, DamageCauser);
	}

	return bCanHitTarget;
}

bool UGunsmithKnockbackDamageFormat::GetUsesChaos() const
{
	if (const UWorld* World = GetWorld())
	{
		if (const UGSWorldStateSubsystem* WorldStateSubsystem = World->GetSubsystem<UGSWorldStateSubsystem>())
		{
			return WorldStateSubsystem->GetUsesAsyncPhysics();
		}
	}

	return false;
}

bool UGunsmithKnockbackDamageFormat::IsForceOnlyTarget(const AActor* Target) const
{
	// Only apply forces to non-movers in chaos
	if (!GetUsesChaos())
	{
		return false;
	}
	
	if (!IsValid(Target) || Target->IsA<AGunsmithMoverCharacter>() || UGSGameplayLibrary::GetHealthComponentFromActor(Target))
	{
		return false;
	}

	return FindSimulatingComponent(Target) != nullptr;
}

UPrimitiveComponent* UGunsmithKnockbackDamageFormat::FindSimulatingComponent(const AActor* Actor,
	UPrimitiveComponent* PreferredComponent)
{
	if (IsValid(PreferredComponent) && PreferredComponent->GetOwner() == Actor && PreferredComponent->IsSimulatingPhysics())
	{
		return PreferredComponent;
	}

	TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(Actor);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (PrimitiveComponent->IsSimulatingPhysics())
		{
			return PrimitiveComponent;
		}
	}

	return nullptr;
}
