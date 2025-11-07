// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpringArmComponent.h"
#include "GunsmithSpringArmComponent.generated.h"

struct FCollisionQueryParams;

/**
 *	A custom spring arm component required to ignore spectator pawns targets
 */
UCLASS()
class GUNSMITHEXAMPLES_API UGunsmithSpringArmComponent : public USpringArmComponent
{
	GENERATED_BODY()

public:
	// USpringArmComponent Begin
	virtual FVector BlendLocations(const FVector& DesiredArmLocation, const FVector& TraceHitLocation, bool bHitSomething, float DeltaTime) override;
	// USpringArmComponent End
	
	// Sets the component rotation and updates the cached rotation value to match
	void SetSpringArmRelativeRotation(const FRotator& Rotation);

	TOptional<float> GetLastHitDistance() const { return LastHitDistance; }
	
protected:
	// USpringArmComponent Begin
	virtual void UpdateDesiredArmLocation(bool bDoTrace, bool bDoLocationLag, bool bDoRotationLag, float DeltaTime) override;
	// USpringArmComponent End

	// Append params to the trace query so that more actors can be ignored
	void AddCustomParamsToTrace(FCollisionQueryParams& QueryParams) const;

	TOptional<float> LastHitDistance = 0.0f;
};
