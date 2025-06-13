// Copyright Kraken Party Limited. All Rights Reserved.


#include "Game/GunsmithPickupActor.h"

#include "GSGameplayLibrary.h"
#include "Components/SphereComponent.h"
#include "Netcode/GSNetworkLibrary.h"

AGunsmithPickupActor::AGunsmithPickupActor()
{
	SphereComponent = CreateDefaultSubobject<USphereComponent>("TriggerSphere");
	SphereComponent->SetSphereRadius(20.0f);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SetRootComponent(SphereComponent);

	WeaponMesh = CreateDefaultSubobject<UGSWeaponSkeletalMeshComponent>("WeaponMesh");
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetupAttachment(SphereComponent);
	
	bReplicates = true;
	
	OnActorBeginOverlap.AddDynamic(this, &AGunsmithPickupActor::OnActorOverlap);
}

void AGunsmithPickupActor::BeginPlay()
{
	Super::BeginPlay();

	// Initialise the WeaponMesh with the EquipData
	if (EquipData.WeaponData)
	{
		WeaponMesh->Init(EquipData.WeaponData, EquipData.Skin, EquipData.GatherAttachmentTags());
	}
}

void AGunsmithPickupActor::OnActorOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	// Reject the pickup if bOnlyPickupOnServer is true and this is not on the server
	if (bOnlyPickupOnServer && !UGSNetworkLibrary::HasAuthority(this))
	{
		return;
	}

	// Equip the weapon if the target actor has a shooting component
	if (UGSShootingComponent* ShootingComponent = UGSGameplayLibrary::GetShootingComponentFromActor(OtherActor))
	{
		ShootingComponent->EquipWeapon(EquipData);

		// Destroy the pickup actor
		if (bDestroyOnPickup)
		{
			Destroy(true);
		}
	}
}
