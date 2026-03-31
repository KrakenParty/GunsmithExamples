// Copyright Kraken Party Limited. All Rights Reserved.


#include "Game/Weapon/Behavior/GunsmithClosestTargetBehavior.h"

#include "GunsmithMoverCharacter.h"
#include "Character/GSSkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Weapon/GSShootingComponent.h"


UGunsmithClosestTargetBehavior::UGunsmithClosestTargetBehavior()
{
	BehaviorTimeline = EGSTargetBehaviorTimeline::SimulatedProxy;
}

void UGunsmithClosestTargetBehavior::CacheTargetData_Implementation(int32 Frame,
                                                                    const UGSShootingComponent* InShootingComponent, const UGSShootingTickStartData* SimInput,
                                                                    const UGSShootingTickEndData* SimOutput, const FRotator& AuthoritativeLookAtRotation)
{
	const FGSDefaultShootingInputs* DefaultShootingInputs = SimInput->GetInputState().DataCollection.FindDataByType<FGSDefaultShootingInputs>();
	
	if (!ShootingComponent.IsValid() || !DefaultShootingInputs)
	{
		return;
	}
	
	const FVector EyesLocation = GetEyesLocation(SimInput);
	const FVector LookRotationForward = DefaultShootingInputs->LookRotation.Vector();
	
	// Find the best target
	TArray<FGunsmithPotentialTargetData> PotentialTargets = GetPotentialTargets(SimInput, SimOutput);
	
	FGunsmithPotentialTargetData BestTarget;
	float BestTargetWeight = 0.0f;
	for (const FGunsmithPotentialTargetData& PotentialTarget : PotentialTargets)
	{
		const int32 CurrentWeight = GetTargetWeight(PotentialTarget, SimInput, SimOutput);
		
		if (!BestTarget.TargetActor || CurrentWeight > BestTargetWeight)
		{
			BestTarget = PotentialTarget;
			BestTargetWeight = CurrentWeight;
		}
	}
	
	// Set the target data
	FGSShooterData ShooterData = CreateShooterData(SimInput, SimOutput);
	ShooterData.EmitterOutputStart = EyesLocation;
	
	FGSTargetData TargetData;
	TargetData.InterpolationID = 0;
	
	// If a target has been selected
	if (BestTarget.TargetActor)
	{
		TargetData.EmitterOutputEnd = BestTarget.TargetLocation;
		TargetData.AimDistanceOverride = (TargetData.EmitterOutputEnd - ShooterData.EmitterOutputStart).Length();
	}
	else // No valid target
	{
		TargetData.AimDistanceOverride = AimTraceDistance;
		TargetData.EmitterOutputEnd = EyesLocation + LookRotationForward * AimTraceDistance;
	}
	
	SetShooterDataForFrame(Frame, ShooterData);
	SetTargetDataForFrame(Frame, { TargetData });
}

bool UGunsmithClosestTargetBehavior::CanTargetActor_Implementation(const FGunsmithPotentialTargetData& PotentialTarget, const UGSShootingTickStartData* SimInput, const UGSShootingTickEndData* SimOutput) const
{
	const FGSDefaultShootingInputs* DefaultShootingInputs = SimInput->GetInputState().DataCollection.FindDataByType<FGSDefaultShootingInputs>();
	const FVector EyesLocation = GetEyesLocation(SimInput);
	const FQuat QuatToTarget = (PotentialTarget.TargetLocation - EyesLocation).ToOrientationQuat();
	
	// Is target within max angle
	const float Distance = DefaultShootingInputs->LookRotation.Quaternion().AngularDistance(QuatToTarget);
	if (FMath::RadiansToDegrees(Distance) > MaxTargetAngle)
	{
		return false;
	}
	
	// Is target visible (This is really naive)
	FCollisionQueryParams QueryParams = CreateTraceParams(ShootingComponent->GetOwner());
	QueryParams.AddIgnoredActor(PotentialTarget.TargetActor);
	const ECollisionChannel CollisionChannel = bOverrideCollisionChannel ? OverrideTraceCollisionChannel : UGSDeveloperSettings::Get()->DefaultTargetTraceCollisionChannel;
	
	TArray<FHitResult> HitResults;
	if (UGSWeaponBlueprintFunctionLibrary::TraceWithRadius(GetWorld(), HitResults, EyesLocation, PotentialTarget.TargetLocation, CollisionChannel, 0.0f, QueryParams))
	{
		return false;
	}
	
	return true;
}

TArray<FGunsmithPotentialTargetData> UGunsmithClosestTargetBehavior::GetPotentialTargets_Implementation(const UGSShootingTickStartData* SimInput, const UGSShootingTickEndData* SimOutput) const
{
	UWorld* World = GetWorld();
	
	if (!World || !ShootingComponent.IsValid())
	{
		return {};
	}
	
	AGameStateBase* GameState = World->GetGameState();
	
	if (!IsValid(GameState))
	{
		return {};
	}
	
	AActor* OwningActor = ShootingComponent->GetOwner();
	
	TArray<FGunsmithPotentialTargetData> PotentialTargetData;
	PotentialTargetData.Reserve(GameState->PlayerArray.Num());
	for (APlayerState* Player : GameState->PlayerArray)
	{
		AGunsmithMoverCharacter* PlayerPawn = Cast<AGunsmithMoverCharacter>(Player->GetPawn());
		
		if (IsValid(PlayerPawn) && PlayerPawn != OwningActor)
		{
			UGSSkeletalMeshComponent* PlayerMesh = PlayerPawn->GetMesh();
			
			FGunsmithPotentialTargetData PotentialTarget;
			PotentialTarget.TargetActor = PlayerPawn;
			PotentialTarget.TargetLocation = PlayerMesh ? PlayerMesh->GetBoneTransform(TargetBoneName).GetLocation() : PlayerPawn->GetActorLocation();
			
			if (CanTargetActor(PotentialTarget, SimInput, SimOutput))
			{
				PotentialTargetData.Emplace(PotentialTarget);
			}
		}
	}
	
	return PotentialTargetData;
}

float UGunsmithClosestTargetBehavior::GetTargetWeight_Implementation(const FGunsmithPotentialTargetData& Target, const UGSShootingTickStartData* SimInput, const UGSShootingTickEndData* SimOutput) const
{
	// For now, just prefer the closest target
	const FVector EyesLocation = GetEyesLocation(SimInput);
	
	return AimTraceDistance - (Target.TargetLocation - EyesLocation).Length();
}