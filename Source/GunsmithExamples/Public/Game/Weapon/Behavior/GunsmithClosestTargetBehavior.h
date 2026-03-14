// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Target/GSStandardTargetBehavior.h"
#include "Weapon/Target/GSTargetBehavior.h"
#include "GunsmithClosestTargetBehavior.generated.h"

USTRUCT(BlueprintType)
struct FGunsmithPotentialTargetData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, Category = "Target Data")
	AActor* TargetActor = nullptr;
	
	UPROPERTY(BlueprintReadWrite, Category = "Target Data")
	FVector TargetLocation = FVector::ZeroVector;
};

/**
 * A custom target behavior that returns the closest target to the player
 */
UCLASS()
class GUNSMITHEXAMPLES_API UGunsmithClosestTargetBehavior : public UGSStandardTargetBehavior
{
	GENERATED_BODY()
	
public:
	UGunsmithClosestTargetBehavior();
	
protected:	
	// UGSTargetBehavior Begin
	virtual void CacheTargetData_Implementation(int32 Frame, const UGSShootingComponent* InShootingComponent, const UGSShootingTickStartData* SimInput, const UGSShootingTickEndData* SimOutput, const FRotator& AuthoritativeLookAtRotation) override;
	// UGSTargetBehavior End
	
	// Return a list of potential targets which will later get sorted by weight
	UFUNCTION(BlueprintNativeEvent, Category = "Target Data")
	TArray<FGunsmithPotentialTargetData> GetPotentialTargets(const UGSShootingTickStartData* SimInput, const UGSShootingTickEndData* SimOutput) const;
	
	// Return true if the actor can be targeted
	UFUNCTION(BlueprintNativeEvent, Category = "Target Data")
	bool CanTargetActor(const FGunsmithPotentialTargetData& PotentialTarget, const UGSShootingTickStartData* SimInput, const UGSShootingTickEndData* SimOutput) const;
	
	// Returns a value which is used to find the best target. The highest weight value will be selected
	UFUNCTION(BlueprintNativeEvent, Category = "Target Data")
	float GetTargetWeight(const FGunsmithPotentialTargetData& Target, const UGSShootingTickStartData* SimInput, const UGSShootingTickEndData* SimOutput) const;
	
	// Which bone on the target mesh should we aim for
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Target Data")
	FName TargetBoneName = "spine_05";
	
	// What angular distance should the target be within
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Target Data")
	float MaxTargetAngle = 10.0f;
};
