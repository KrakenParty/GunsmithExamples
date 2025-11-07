// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Netcode/GSRollbackInterface.h"
#include "Weapon/Target/GSDamageTargetInterface.h"
#include "GunsmithRangeTargetActor.generated.h"

class UGSRollbackComponent;
class UStaticMeshComponent;
class UGSHealthComponent;
class UGunsmithTargetMovementComponent;
struct FGSDamageRecord;

DECLARE_MULTICAST_DELEGATE_OneParam(FGunsmithTargetActiveStateChangedDelegate, bool bActive);

UCLASS(Blueprintable)
class GUNSMITHEXAMPLES_API AGunsmithRangeTargetActor : public AActor, public IGSDamageTargetInterface, public IGSRollbackInterface
{
	GENERATED_BODY()

public:
	AGunsmithRangeTargetActor();

	void SetTargetActive(bool bActive) const;
	bool IsActive() const;

	FGunsmithTargetActiveStateChangedDelegate OnActiveStateChanged;
	
protected:
	// AActor Begin
	virtual void BeginPlay() override;
	// AActor End

	// IGSDamageTargetInterface Begin
	virtual UGSHealthComponent* GetHealthComponent_Implementation() const override;
	virtual float GetDamageMultiplierForHitComponent_Implementation(const UPrimitiveComponent* Component) const override;
	// IGSDamageTargetInterface End

	// IGSRollbackInterface Begin
	virtual UGSRollbackComponent* GetRollbackComponent_Implementation() const override;
	// IGSRollbackInterface End

	/*** Collision ***/
	
	// Which collision profile should be used for rollback colliders
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
	FName RollbackCollisionProfileName;

	// Which collision channel object type should be used for rollback colliders
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
	TEnumAsByte<ECollisionChannel> RollbackCollisionObjectType = ECC_Pawn;

	/*** Components ***/
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rollback")
	TObjectPtr<UGSRollbackComponent> RollbackComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	TObjectPtr<UGSHealthComponent> HealthComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rollback")
	TObjectPtr<UStaticMeshComponent> StickMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rollback")
	TObjectPtr<UStaticMeshComponent> TargetMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rollback")
	TObjectPtr<UGunsmithTargetMovementComponent> MovementComponent = nullptr;

private:	
	UFUNCTION()
	void OnDeath(UGSHealthComponent* AffectedHealthComponent, const FGSDamageRecord& DamageRecord, bool bIsPredicted);
};
