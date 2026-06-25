// Copyright Kraken Party Limited. All Rights Reserved.


#include "Game/GunsmithShootingActor.h"

#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Character/GSCharacter.h"
#include "Netcode/GSNetworkLibrary.h"
#include "Weapon/GSShootingComponent.h"

AGunsmithShootingActor::AGunsmithShootingActor()
{
	bReplicates = true;
	
	PrimaryActorTick.bCanEverTick = true;
	
	ShootingComponent = CreateDefaultSubobject<UGSShootingComponent>(TEXT("ShootingComponent"));
}

void AGunsmithShootingActor::BeginPlay()
{
	ShootingComponent->SetInputProducer(this);
	
	Super::BeginPlay();
}

void AGunsmithShootingActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// For now, just target the first GS Character
	if (!TargetPawn.IsValid() && UGSNetworkLibrary::HasAuthority(this))
	{
		for (TActorIterator<AGSCharacter> It(GetWorld()); It; ++It)
		{
			if (AGSCharacter* CharacterToDestroy = *It)
			{
				TargetPawn = CharacterToDestroy;
			}
		}
	}
}

void AGunsmithShootingActor::ProduceShootingInput_Implementation(UGSShootingComponent* TargetShootingComponent, int32 SimTimeMs, FGSShootingInputState& InputCmdResult)
{
	FGSDefaultShootingInputs& DefaultInputs = InputCmdResult.DataCollection.FindOrAddMutableDataByType<FGSDefaultShootingInputs>();
	FGSDefaultCameraInputs& DefaultCameraInputs = InputCmdResult.DataCollection.FindOrAddMutableDataByType<FGSDefaultCameraInputs>();

	DefaultInputs.bIsShooting = TargetPawn.IsValid();
	
	if (DefaultInputs.bIsShooting)
	{
		FVector TargetLocation = TargetPawn->GetActorLocation();
		TargetLocation.Z += TargetPawn->GetDefaultHalfHeight() / 2;

		DefaultCameraInputs.CameraLocation = GetActorLocation();
		DefaultCameraInputs.LookRotation = (TargetLocation - DefaultCameraInputs.CameraLocation).Rotation();
	}
}

TArray<USceneComponent*> AGunsmithShootingActor::GetWeaponAttachmentComponents_Implementation() const
{
	return { RootComponent };
}
