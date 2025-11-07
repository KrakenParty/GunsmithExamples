// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/Modes/Range/GunsmithRangeTargetActor.h"

#include "GSDeveloperSettings.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Game/Modes/Range/GunsmithRangeGameMode.h"
#include "Game/Modes/Range/GunsmithTargetMovementComponent.h"
#include "Netcode/GSRollbackComponent.h"

namespace GunsmithRangeTargetNames
{
	static const FName HealthComponent = TEXT("HealthComponent");
	static const FName RollbackComponent = TEXT("RollbackComponent");
	static const FName MovementComponent = TEXT("MovementComponent");
	static const FName StickMesh = TEXT("StickMesh");
	static const FName TargetMesh = TEXT("TargetMesh");
};

AGunsmithRangeTargetActor::AGunsmithRangeTargetActor()
{
	TargetMesh = CreateDefaultSubobject<UStaticMeshComponent>(GunsmithRangeTargetNames::TargetMesh);
	SetRootComponent(TargetMesh);
	
	StickMesh = CreateDefaultSubobject<UStaticMeshComponent>(GunsmithRangeTargetNames::StickMesh);
	StickMesh->SetupAttachment(TargetMesh);
	
	RollbackComponent = CreateDefaultSubobject<UGSRollbackComponent>(GunsmithRangeTargetNames::RollbackComponent);

	HealthComponent = CreateDefaultSubobject<UGSHealthComponent>(GunsmithRangeTargetNames::HealthComponent);

	MovementComponent = CreateDefaultSubobject<UGunsmithTargetMovementComponent>(GunsmithRangeTargetNames::MovementComponent);

	bReplicates = true;
}

void AGunsmithRangeTargetActor::SetTargetActive(bool bActive) const
{
	if (bActive == MovementComponent->GetIsActive())
	{
		return;
	}
	
	MovementComponent->SetTargetActive(bActive);

	if (bActive)
	{
		HealthComponent->SetHealth(HealthComponent->GetMaxHealth());
	}

	OnActiveStateChanged.Broadcast(bActive);
}

bool AGunsmithRangeTargetActor::IsActive() const
{
	return MovementComponent->GetIsActive();
}

void AGunsmithRangeTargetActor::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->OnDeath.AddUniqueDynamic(this, &AGunsmithRangeTargetActor::OnDeath);

	AGSRollbackProxy* RollbackProxy = RollbackComponent->GetRollbackProxy();
	if (RollbackProxy)
	{
		if (TargetMesh)
		{
			RollbackProxy->SetupTrackedMesh(TargetMesh, RollbackCollisionProfileName, RollbackCollisionObjectType);

			UPhysicalMaterial* PhysicalMaterial = TargetMesh->BodyInstance.GetSimplePhysicalMaterial();
			RollbackProxy->SetupStaticMesh(GunsmithRangeTargetNames::StickMesh, StickMesh, StickMesh->GetStaticMesh(), FVector::ZeroVector, FRotator::ZeroRotator, false, PhysicalMaterial);
			RollbackProxy->SetupStaticMesh(GunsmithRangeTargetNames::TargetMesh, TargetMesh, TargetMesh->GetStaticMesh(), FVector::ZeroVector, FRotator::ZeroRotator, false, PhysicalMaterial);
		}
	}

	if (AGunsmithRangeGameMode* RangeGameMode = GetWorld()->GetAuthGameMode<AGunsmithRangeGameMode>())
	{
		RangeGameMode->RegisterTargetActor(this);
	}
}

UGSHealthComponent* AGunsmithRangeTargetActor::GetHealthComponent_Implementation() const
{
	return HealthComponent;
}

float AGunsmithRangeTargetActor::GetDamageMultiplierForHitComponent_Implementation(
	const UPrimitiveComponent* Component) const
{
	if (AGSRollbackProxy* RollbackProxy = RollbackComponent->GetRollbackProxy())
	{
		if (Component == RollbackProxy->GetColliderWithName(GunsmithRangeTargetNames::TargetMesh))
		{
			return 1.0f;
		}
	}
	
	return 0.0f;
}

UGSRollbackComponent* AGunsmithRangeTargetActor::GetRollbackComponent_Implementation() const
{
	return RollbackComponent;
}

void AGunsmithRangeTargetActor::OnDeath(UGSHealthComponent* AffectedHealthComponent, const FGSDamageRecord& DamageRecord,
                                        bool bIsPredicted)
{
	SetTargetActive(false);
}
