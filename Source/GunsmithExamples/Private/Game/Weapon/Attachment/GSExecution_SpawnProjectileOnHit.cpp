// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/Weapon/Attachment/GSExecution_SpawnProjectileOnHit.h"

#include "World/GSWorldStateSubsystem.h"
#include "NetworkPredictionWorldManager.h"
#include "Netcode/GSRollbackProxy.h"
#include "Weapon/Emitter/Output/Projectile/GSProjectileDataAsset.h"
#include "Weapon/GSShootingComponent.h"
#include "Weapon/Emitter/GSWeaponProjectileEmitter.h"
#include "Weapon/Target/GSDamageTargetInterface.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_Weapon_Attribute_Attachment_SpawnProjectileOnHit_NumProjectiles, "Weapon.Attribute.SpawnProjectileOnHit.NumProjectiles")
UE_DEFINE_GAMEPLAY_TAG(TAG_Weapon_Attribute_Attachment_SpawnProjectileOnHit_SpreadRadius, "Weapon.Attribute.SpawnProjectileOnHit.SpreadRadius")

void UGSExecution_SpawnProjectileOnHit::Setup(UGSShootingComponent* ShootingComponent, int32 ID)
{
	Super::Setup(ShootingComponent, ID);

	NumProjectiles.Register(this, ShootingComponent);
	ProjectileSpreadRadius.Register(this, ShootingComponent);
	
	if (ProjectileToSpawn)
	{
		ShootingComponent->AddEmitterOutputDataInstance(ProjectileToSpawn, this);

#if WITH_EDITOR
		if (!BlueprintCompiledHandle.IsValid())
		{
			if (UBlueprint* BlueprintAsset = Cast<UBlueprint>(ProjectileToSpawn->GetClass()->ClassGeneratedBy))
			{
				BlueprintCompiledHandle = BlueprintAsset->OnCompiled().AddUObject(ShootingComponent, &UGSShootingComponent::OnEmitterOutputDataBlueprintChanged);
			}
		}
#endif
	}
}

void UGSExecution_SpawnProjectileOnHit::Apply(UGSShootingComponent* ShootingComponent, const FGSEquipData& CurrentWeapon, int32 Frame)
{
	if (!ProjectileToSpawn)
	{
		return;
	}
	
	Super::Apply(ShootingComponent, CurrentWeapon, Frame);

	ActivatedFrame = Frame;

	UGSWorldStateSubsystem* WorldStateSubsystem = ShootingComponent->GetWorld()->GetSubsystem<UGSWorldStateSubsystem>();
	if (ensure(WorldStateSubsystem) && NumProjectiles.GetModifiedValue() > 0)
	{
		if (AActor* Owner = ShootingComponent->GetOwner())
		{
			WorldStateSubsystem->RegisterProjectileCreatedDelegate(Owner).AddDynamic(this, &UGSExecution_SpawnProjectileOnHit::OnProjectileSpawned);
		}
	}
}

void UGSExecution_SpawnProjectileOnHit::Remove(UGSShootingComponent* ShootingComponent)
{
	Super::Remove(ShootingComponent);

	AActor* Instigator = ShootingComponent->GetOwner();
	if (Instigator)
	{
		UGSWorldStateSubsystem* WorldStateSubsystem = Instigator->GetWorld()->GetSubsystem<UGSWorldStateSubsystem>();
		if (WorldStateSubsystem)
		{
			WorldStateSubsystem->ClearProjectileCreatedDelegate(Instigator, [this](FGSProjectileCreatedDelegate& Delegate)
			{
				Delegate.RemoveDynamic(this, &UGSExecution_SpawnProjectileOnHit::OnProjectileSpawned);
			});
		}
	}

#if WITH_EDITOR
	if (BlueprintCompiledHandle.IsValid())
	{
		if (UBlueprint* BlueprintAsset = Cast<UBlueprint>(ProjectileToSpawn->GetClass()->ClassGeneratedBy))
		{
			BlueprintAsset->OnCompiled().Remove(BlueprintCompiledHandle);
			BlueprintCompiledHandle.Reset();
		}
	}
#endif
}

UGSEmitterOutputDataAsset* UGSExecution_SpawnProjectileOnHit::GetAssociatedEmitterOutputDataAsset() const
{
	return ProjectileToSpawn;
}

void UGSExecution_SpawnProjectileOnHit::OnProjectileSpawned(UGSProjectileState* Projectile)
{
	if (Projectile && DoesProjectileMatchFilter(Projectile))
	{
		Projectile->OnProjectileHitTarget.AddDynamic(this, &UGSExecution_SpawnProjectileOnHit::OnProjectileHitTarget);
	}
}

void UGSExecution_SpawnProjectileOnHit::OnProjectileHitTarget(int32 Frame, const FHitResult& Hit,
	 UGSProjectileState* ProjectileState, const FGSProjectileFrameState& FrameState)
{
	if (bExecutionEnded)
	{
		return;
	}
	
	check(ProjectileState);

	AActor* Instigator = ProjectileState->GetInstigator();

	if (!Instigator)
	{
		return;
	}

	if (!CanSpawnOnTargetHit(Hit, ProjectileState))
	{
		return;
	}
	
	UNetworkPredictionWorldManager* NetworkPredictionWorldManager = Instigator->GetWorld()->GetSubsystem<UNetworkPredictionWorldManager>();
	UGSWorldStateSubsystem* WorldStateSubsystem = Instigator->GetWorld()->GetSubsystem<UGSWorldStateSubsystem>();
	if (ensure(WorldStateSubsystem))
	{
		// Gather interpolation stats from the NPP world state
		int32 PendingFrame = Frame;
		int32 InterpolatedFrame = Frame;
		float InterpolatedPercentage = 0.0f;

		if (NetworkPredictionWorldManager && !GetWorld()->IsNetMode(NM_DedicatedServer))
		{
			const FFixedTickState& FixedTickState = NetworkPredictionWorldManager->GetFixedTickState();
			InterpolatedFrame = FixedTickState.Interpolation.ToFrame >= 0 ? FixedTickState.Interpolation.ToFrame - 1 : FixedTickState.PendingFrame;
			InterpolatedPercentage = FixedTickState.Interpolation.PCT;	
		}

		const int32 RandomSeed = FrameState.ProjectileID + FrameState.NumBounces + GetExecutionUniqueID();
		FRandomStream RandStream(RandomSeed);

		const float SpreadRadius = ProjectileSpreadRadius.GetModifiedValue();
		
		for (int32 i = 0; i < NumProjectiles.GetModifiedValue(); i++)
		{			
			// Create random direction based on the attachment random seed
			const float RandX = RandStream.RandRange(-SpreadRadius, SpreadRadius);
			const float RandY = RandStream.RandRange(-SpreadRadius, SpreadRadius);
			const FRotator RandomRotator = FRotator(RandX, RandY, 0.0f);
			const FQuat ProjectileDirection = RandomRotator.RotateVector(Hit.ImpactNormal).ToOrientationQuat();

			FGSEmitterEventCue EmitterCue;
			EmitterCue.EventID = FrameState.ProjectileID;
			
			FGSDefaultEmitterEventData& SetupData = EmitterCue.EventData.FindOrAddMutableDataByType<FGSDefaultEmitterEventData>();
			SetupData.AttachmentID = GetExecutionUniqueID();
			SetupData.SimulationFrame = PendingFrame;
			SetupData.SimulatedProxyFrame = InterpolatedFrame;
			SetupData.InterpolatedFrameRemainder = InterpolatedPercentage;
			SetupData.StartLocation = Hit.ImpactPoint;
			SetupData.StartDirection = ProjectileDirection;

			FGSProjectileEmitterEventData& ProjectileData = EmitterCue.EventData.FindOrAddMutableDataByType<FGSProjectileEmitterEventData>();
			ProjectileData.InitialStrength = 1.0f;
		
			UGSProjectileState* Projectile = WorldStateSubsystem->CreateProjectile(Instigator, EmitterCue, ProjectileToSpawn, FTransform(ProjectileDirection, SetupData.StartLocation));
			// Make sure the new projectile doesn't hit the ground in the same place the last hit was recorded
			Projectile->GetActiveFrameData().ComponentHitLastFrame = Hit.GetComponent();
		}
	}
}

bool UGSExecution_SpawnProjectileOnHit::CanSpawnOnTargetHit(const FHitResult& Hit, UGSProjectileState* ProjectileState) const
{
	FHitResult HitCopy = Hit;

	// Prevent spawning if unable to bounce on the target
	if (bIgnoreSpawningIfCannotBounceOnTarget)
	{
		const AActor* HitActor = HitCopy.GetActor();
		if (IsValid(HitActor) && HitActor->GetClass()->ImplementsInterface(UGSDamageTargetInterface::StaticClass()))
		{
			if (!IGSDamageTargetInterface::Execute_CanEmitterOutputBounceOff(HitActor, ProjectileState->GetProjectileDataAsset()))
			{
				return false;
			}
		}
	}

	return true;
}

bool UGSExecution_SpawnProjectileOnHit::DoesProjectileMatchFilter(UGSProjectileState* Projectile) const
{
	const UGSProjectileDataAsset* ProjectileDataAsset = Projectile->GetProjectileDataAsset();

	if (ActivatedFrame != -1 && Projectile->GetStartFrame() < ActivatedFrame)
	{
		return false;
	}
	
	if (!ProjectileTagFilter.IsEmpty() && (!ProjectileDataAsset || !ProjectileTagFilter.HasTag(ProjectileDataAsset->AssetTag)))
	{
		return false;
	}

	return true;
}

#if WITH_EDITOR
void UGSExecution_SpawnProjectileOnHit::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property != nullptr)
	{
		const FName PropertyChanged = PropertyChangedEvent.Property->GetFName();
		// Update bindings for projectile to spawn
		if (PropertyChanged == GET_MEMBER_NAME_CHECKED(UGSExecution_SpawnProjectileOnHit, ProjectileToSpawn))
		{
			if (UBlueprint* BlueprintAsset = Cast<UBlueprint>(ProjectileToSpawn->GetClass()->ClassGeneratedBy))
			{
				if (BlueprintCompiledHandle.IsValid())
				{
					BlueprintAsset->OnCompiled().Remove(BlueprintCompiledHandle);
					BlueprintCompiledHandle.Reset();
				}

				if (UGSShootingComponent* ShootingComponent = Cast<UGSShootingComponent>(GetOuter()))
				{
					BlueprintCompiledHandle = BlueprintAsset->OnCompiled().AddUObject(ShootingComponent, &UGSShootingComponent::OnEmitterOutputDataBlueprintChanged);	
				}
			}
		}
	}
}
#endif
