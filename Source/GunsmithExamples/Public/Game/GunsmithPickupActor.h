// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/GSShootingComponent.h"
#include "GameFramework/Actor.h"
#include "GunsmithPickupActor.generated.h"

class USphereComponent;

/**
 * An example of how to create a pickup actor with Gunsmith. Also uses UGSWeaponSkeletalMeshComponent to display the gun mesh dynamically
 */
UCLASS(Blueprintable)
class GUNSMITHEXAMPLES_API AGunsmithPickupActor : public AActor
{
	GENERATED_BODY()
	
public:
	// AActor Begin
	AGunsmithPickupActor();

	virtual void BeginPlay() override;
	// AActor End

protected:
	/*** Exposed Variables ***/

	// The equipment data that should be applied to the shooting component on overlap
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	FGSEquipData EquipData;

	// If true, this actor is destroyed when the pickup is equipped
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	bool bDestroyOnPickup = false;

	// If true, the equipment will only be applied on the server
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	bool bOnlyPickupOnServer = false;

	/*** Components  ***/

	// The root sphere component responsible for triggering the overlap
	UPROPERTY(EditAnywhere)
	TObjectPtr<USphereComponent> SphereComponent = nullptr;

	// The weapon visual which is initialised on BeginPlay with the EquipData
	UPROPERTY(EditAnywhere)
	TObjectPtr<UGSWeaponSkeletalMeshComponent> WeaponMesh = nullptr;

private:
	// Called when an actor overlaps the SphereComponent
	UFUNCTION()
	void OnActorOverlap(AActor* OverlappedActor, AActor* OtherActor);
};
