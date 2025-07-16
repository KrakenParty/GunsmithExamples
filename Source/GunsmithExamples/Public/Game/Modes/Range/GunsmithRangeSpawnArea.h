// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GunsmithRangeSpawnArea.generated.h"

class UBoxComponent;

UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithRangeSpawnArea : public AActor
{
	GENERATED_BODY()
	
public:	
	AGunsmithRangeSpawnArea();

	// AActor Begin
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	// AActor End

	FVector GetRandomSpawnLocation(float MaxRangeOverride, OUT float& OutSpawnDistance) const;

protected:
	UPROPERTY(EditAnywhere, Category = "Spawn Area")
	TObjectPtr<UStaticMeshComponent> CenterLine = nullptr;

	UPROPERTY(EditAnywhere, Category = "Spawn Area")
	TObjectPtr<UStaticMeshComponent> LeftLine = nullptr;

	UPROPERTY(EditAnywhere, Category = "Spawn Area")
	TObjectPtr<UStaticMeshComponent> RightLine = nullptr;

	UPROPERTY(EditAnywhere, Category = "Spawn Area", meta=(ClampMin="0"))
	float AreaAngleInDegrees = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Spawn Area", meta=(ClampMin="0"))
	float MaxDistance = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Spawn Area")
	float SpawnZOffset = 100.0f;
};
