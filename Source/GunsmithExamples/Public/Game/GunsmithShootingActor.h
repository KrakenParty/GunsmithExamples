// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon/GSEquipData.h"
#include "Weapon/GSShootingInterface.h"
#include "GunsmithShootingActor.generated.h"

/*
 *	An example of a non-pawn actor using the Gunsmith shooting component
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithShootingActor : public AActor, public IGSShootingInterface
{
	GENERATED_BODY()
	
public:	
	AGunsmithShootingActor();

	// AActor Begin
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	// AActor End

	// IGSShootingInterface Begin
	virtual UGSShootingComponent* GetShootingComponent_Implementation(const FGameplayTag Tag) const override { return ShootingComponent; }
	virtual void ProduceShootingInput_Implementation(UGSShootingComponent* TargetShootingComponent, int32 SimTimeMs, FGSShootingInputState& InputCmdResult) override;
	virtual TArray<USceneComponent*> GetWeaponAttachmentComponents_Implementation() const override;
	// IGSShootingInterface End

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooting")
	TObjectPtr<UGSShootingComponent> ShootingComponent = nullptr;

	TWeakObjectPtr<APawn> TargetPawn;
};
