// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/GunsmithSpectatorPawn.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GSGameplayLibrary.h"
#include "AI/NavigationSystemBase.h"
#include "Camera/CameraComponent.h"
#include "Engine/LocalPlayer.h"
#include "Game/GunsmithSpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Health/GSHealthComponent.h"
#include "Health/GSHealthViewModel.h"
#include "Kismet/GameplayStatics.h"
#include "UI/GunsmithHUD.h"
#include "UObject/UObjectIterator.h"
#include "Weapon/GSShootingComponent.h"

AGunsmithSpectatorPawn::AGunsmithSpectatorPawn()
{
	bAddDefaultMovementBindings = false;
	bUseControllerRotationYaw = true;
	AIControllerClass = nullptr;

	SpringArmComponent = CreateDefaultSubobject<UGunsmithSpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 200.0f;
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f));

	PrimaryActorTick.bCanEverTick = true;
}

void AGunsmithSpectatorPawn::BeginPlay()
{
	Super::BeginPlay();

	DesiredTargetArmLength = SpringArmComponent->TargetArmLength;
}

void AGunsmithSpectatorPawn::SetupPlayerInputComponent(UInputComponent* InInputComponent)
{
	Super::SetupPlayerInputComponent(InInputComponent);

	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InInputComponent))
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				if (InputMappingContext)
				{
					EnhancedInputSubsystem->AddMappingContext(InputMappingContext, 0);
				}
			}
		}
		
		if (LookInputMouseAction)
		{
			Input->BindAction(LookInputMouseAction, ETriggerEvent::Triggered, this, &AGunsmithSpectatorPawn::OnLookTriggered);
		}

		if (SwitchViewTargetAction)
		{
			Input->BindAction(SwitchViewTargetAction, ETriggerEvent::Started, this, &AGunsmithSpectatorPawn::OnSwitchViewTargetStarted);
		}

		if (ScrollViewAction)
		{
			Input->BindAction(ScrollViewAction, ETriggerEvent::Triggered, this, &AGunsmithSpectatorPawn::OnScrollViewTriggered);
		}
	}
}

void AGunsmithSpectatorPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	SwitchSpectatedPlayer(1);
}

void AGunsmithSpectatorPawn::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
	CameraComponent->GetCameraView(DeltaTime, OutResult);
}

void AGunsmithSpectatorPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// TODO for some reason, the initial snap to actor does not work in replays so this fixes it for now
	// I'll fix it up when I have time to look into it properly
	if (!RootComponent->GetRelativeLocation().IsNearlyZero() && GetAttachParentActor())
	{
		SetActorRelativeLocation(FVector::ZeroVector);
	}

	if (SpringArmComponent->TargetArmLength != DesiredTargetArmLength)
	{
		float NewArmValue = SpringArmComponent->TargetArmLength;
		FMath::SpringDamperSmoothing(NewArmValue, ArmLengthVelocity, DesiredTargetArmLength, ScrollAcceleration * ScrollDirection, DeltaSeconds, ScrollSmoothTime, 1.0f);

		float ClampedValue = FMath::Clamp(NewArmValue, MinScrollDistance, MaxScrollDistance);

		const TOptional<float> LastHitDistance = SpringArmComponent->GetLastHitDistance();
		if (LastHitDistance.IsSet() && LastHitDistance.GetValue() < ClampedValue)
		{
			ClampedValue = LastHitDistance.GetValue();
		}

		// Reset the velocity if we've hit the limit
		if (NewArmValue != ClampedValue)
		{
			ArmLengthVelocity = 0.0f;
		}
		
		SpringArmComponent->TargetArmLength = ClampedValue;
	}
}

void AGunsmithSpectatorPawn::SwitchSpectatedPlayer(int32 Direction)
{
	APawn* NewPawnToSpectate = GetNextSpectatablePawn(Direction);

	if (NewPawnToSpectate == SpectatedPawn)
	{
		return;
	}

	if (SpectatedPawn.IsValid())
	{
		SpectatedPawn->OnDestroyed.RemoveDynamic(this, &AGunsmithSpectatorPawn::OnSpectatedPawnDestroyed);
	}
	
	SpectatedPawn = NewPawnToSpectate;

	if (SpectatedPawn.IsValid())
	{
		SetActorLocation(SpectatedPawn->GetActorLocation());
		AttachToActor(SpectatedPawn.Get(), FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true));

		if (UGSShootingComponent* SpectatedShootingComponent = UGSGameplayLibrary::GetShootingComponentFromActor(SpectatedPawn.Get()))
		{
			const FRotator LookRotation = SpectatedShootingComponent->GetLookRotationFromSimulation();
			const FTransform ParentTransform = SpectatedPawn->GetTransform();

			// TODO This still isn't looking in the right direction unfortunately. Maybe something to do with USpringArmComponent::GetSocketTransform as it's using custom logic with a cached rotation
			FRotator NewRotation = FRotator::ZeroRotator;
			NewRotation.Pitch = 10.0f;
			NewRotation.Yaw = LookRotation.Yaw;
			FTransform NewRelativeTransform = ParentTransform.GetRelativeTransform(FTransform(NewRotation));
			SpringArmComponent->SetSpringArmRelativeRotation(NewRelativeTransform.Rotator());
		}

		SpectatedPawn->OnDestroyed.AddDynamic(this, &AGunsmithSpectatorPawn::OnSpectatedPawnDestroyed);

		// Make sure the HUD points at the new target
		if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
		{
			if (AGunsmithHUD* GunsmithHUD = PlayerController->GetHUD<AGunsmithHUD>())
			{
				GunsmithHUD->SetSpectatedPawn(SpectatedPawn.Get());
			}
		}
	}
}

APawn* AGunsmithSpectatorPawn::GetNextSpectatablePawn(int32 Dir) const
{
	// A modified version of APlayerController::GetNextViewablePlayer that doesn't require the Game Mode to be available to work
	UWorld* World = GetWorld();
	AGameStateBase* GameState = World->GetGameState();

	// Can't continue unless we have the GameState and GameMode
	if (!GameState)
	{
		return nullptr;
	}

	APlayerState* NextPlayerState = (SpectatedPawn.IsValid() ? SpectatedPawn->GetPlayerState() : nullptr);
	
	// If we don't have a NextPlayerState, use our own.
	// This will allow us to attempt to find another player to view or, if all else fails, makes sure we have a playerstate set for next time.
	AController* OurController = GetController();
	int32 NextIndex = (NextPlayerState ? GameState->PlayerArray.Find(NextPlayerState) : GameState->PlayerArray.Find(OurController->PlayerState));

	//Check that NextIndex is a valid index, as Find() may return INDEX_NONE
	if (!GameState->PlayerArray.IsValidIndex(NextIndex))
	{
		if (World->GetDemoNetDriver())
		{
			return GetNextSpectatableDemoPawn(Dir);
		}
		
		return nullptr;
	}

	// Cycle through the player states until we find a valid one.
	for (int32 i = 0; i < GameState->PlayerArray.Num(); ++i)
	{
		NextIndex = ((NextIndex == 0) && (Dir < 0)) ? (GameState->PlayerArray.Num() - 1) : ((NextIndex == (GameState->PlayerArray.Num() - 1)) && (Dir > 0)) ? 0 : NextIndex += Dir;
		NextPlayerState = GameState->PlayerArray[NextIndex];

		// Make sure we're not trying to view our own player state.
		if (NextPlayerState != GetPlayerState())
		{
			// Check they have a pawn
			if (CanSpectate(OurController, NextPlayerState))
			{
				break;
			}
		}
	}

	// If we've failed to find another player to view, we'll be back to our original view target playerstate.
	return NextPlayerState->GetPawn();
}

APawn* AGunsmithSpectatorPawn::GetNextSpectatableDemoPawn(int32 Dir) const
{
	// We need to use TObjectIterator as Player States are not valid during a replay
	TArray<APawn*> Pawns;
	for (TObjectIterator<AGSCharacter> It; It; ++It)
	{
		if (IsValid(*It))
		{
			Pawns.Emplace(*It);
		}
	}

	// Sort the pawns to ensure we iterate over the array in the same way
	Pawns.Sort([](const APawn& Lhs, const APawn& Rhs)
	{
		return Lhs.GetName() < Rhs.GetName();
	});

	APawn* NextPawn = (SpectatedPawn.IsValid() ? SpectatedPawn.Get() : nullptr);
	int32 NextIndex = NextPawn ? Pawns.Find(NextPawn) : 0;

	for (int32 i = 0; i < Pawns.Num(); ++i)
	{
		NextIndex = ((NextIndex == 0) && (Dir < 0)) ? (Pawns.Num() - 1) : ((NextIndex == (Pawns.Num() - 1)) && (Dir > 0)) ? 0 : NextIndex += Dir;
		NextPawn = Pawns[NextIndex];
	}

	return NextPawn;
}

bool AGunsmithSpectatorPawn::CanSpectate(AController* Viewer, const APlayerState* ViewTarget)
{
	if (!ViewTarget)
	{
		return false;
	}

	APawn* Pawn = ViewTarget->GetPawn();

	if (!Pawn)
	{
		return false;
	}

	if (UGSHealthComponent* HealthComponent = UGSGameplayLibrary::GetHealthComponentFromActor(Pawn))
	{
		if (UGSHealthViewModel* HealthViewModel = HealthComponent->GetHealthViewModel())
		{
			if (HealthViewModel->GetHealth() <= 0.0f)
			{
				return false;
			}
		}
	}
	
	return true;
}

void AGunsmithSpectatorPawn::OnLookTriggered(const FInputActionValue& Value)
{
	// Prevent looking around if we're not following a target
	if (!GetAttachParentActor())
	{
		return;
	}
	
	FVector2D LookVector = Value.Get<FVector2D>();

	FRotator NewLookRotation = SpringArmComponent->GetComponentRotation();
	NewLookRotation.Yaw = NewLookRotation.Yaw + LookVector.X * InputConfig.LookInputSpeed.X;
	NewLookRotation.Pitch = FMath::Clamp(NewLookRotation.Pitch + LookVector.Y * InputConfig.LookInputSpeed.Y, InputConfig.InputPitchMin, InputConfig.InputPitchMax);
	
	SpringArmComponent->SetWorldRotation(NewLookRotation);
}

void AGunsmithSpectatorPawn::OnSwitchViewTargetStarted(const FInputActionValue& Value)
{
	const int32 Direction = FMath::RoundToInt(Value.GetMagnitude());

	SwitchSpectatedPlayer(Direction);
}

void AGunsmithSpectatorPawn::OnScrollViewTriggered(const FInputActionValue& Value)
{
	const float ScrollValue = Value.GetMagnitude();

	DesiredTargetArmLength = FMath::Clamp(SpringArmComponent->TargetArmLength - (ScrollValue * ScrollSpeed), MinScrollDistance, MaxScrollDistance);

	if (DesiredTargetArmLength != SpringArmComponent->TargetArmLength)
	{
		ScrollDirection = DesiredTargetArmLength > SpringArmComponent->TargetArmLength ? 1 : -1;
	}
}

void AGunsmithSpectatorPawn::OnSpectatedPawnDestroyed(AActor* DestroyedActor)
{
	SwitchSpectatedPlayer(1);
}
