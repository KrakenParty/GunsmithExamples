// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/Modes/Range/GunsmithRangeSpawnArea.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Game/Modes/Range/GunsmithRangeGameMode.h"

AGunsmithRangeSpawnArea::AGunsmithRangeSpawnArea()
{
	CenterLine = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CenterLine"));
	CenterLine->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetRootComponent(CenterLine);

	LeftLine = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftLine"));
	LeftLine->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftLine->SetupAttachment(CenterLine);

	RightLine = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightLine"));
	RightLine->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightLine->SetupAttachment(CenterLine);
}

void AGunsmithRangeSpawnArea::BeginPlay()
{
	Super::BeginPlay();

	if (AGunsmithRangeGameMode* RangeGameMode = GetWorld()->GetAuthGameMode<AGunsmithRangeGameMode>())
	{
		RangeGameMode->RegisterSpawnArea(this);
	}
}

void AGunsmithRangeSpawnArea::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	LeftLine->SetRelativeRotation(FRotator(0.0f, -AreaAngleInDegrees, 0.0f));
	RightLine->SetRelativeRotation(FRotator(0.0f, AreaAngleInDegrees, 0.0f));

	const float XScale = MaxDistance / 100.0f;
	const FVector CenterScale = FVector(XScale, 1.0f, 1.0f);
	CenterLine->SetRelativeScale3D(CenterScale);
	const FVector SideScale = FVector(XScale / FMath::Sin(FMath::DegreesToRadians(AreaAngleInDegrees)), 1.0f, 1.0f);
	LeftLine->SetWorldScale3D(SideScale);
	RightLine->SetWorldScale3D(SideScale);
}

FVector AGunsmithRangeSpawnArea::GetRandomSpawnLocation(float MaxRangeOverride, float& OutSpawnDistance) const
{
	const FVector ActorLocation = GetActorLocation();
	const float RandomAngle = FMath::RandRange(-AreaAngleInDegrees, AreaAngleInDegrees);
	const FRotator RandomRotator = FRotator(0.0f, RandomAngle, 0.0f);
	const float MaxDistanceToUse = FMath::Min(MaxDistance, FMath::Max(0.0f, MaxRangeOverride));
	OutSpawnDistance = FMath::RandRange(0.0f, MaxDistanceToUse);
	
	FVector RandomLocation = ActorLocation + (RandomRotator.Quaternion() * GetActorForwardVector() * OutSpawnDistance);
	RandomLocation.Z = ActorLocation.Z + SpawnZOffset;

	return RandomLocation;
}
