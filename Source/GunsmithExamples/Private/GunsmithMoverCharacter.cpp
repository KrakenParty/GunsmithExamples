// Copyright Epic Games, Inc. All Rights Reserved.


#include "GunsmithMoverCharacter.h"
#include "Components/InputComponent.h"
#include "DefaultMovementSet/CharacterMoverComponent.h"
#include "MoveLibrary/BasedMovementUtils.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GSDeveloperSettings.h"
#include "Netcode/GSRollbackComponent.h"
#include "InputAction.h"
#include "Weapon/GSShootingComponent.h"
#include "World/GSWorldStateSubsystem.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Camera/CameraComponent.h"
#include "Character/GSMoverComponent.h"
#include "Character/GSMoverDeathMode.h"
#include "Character/GSSkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Game/GunsmithGameState.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Health/GSHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Netcode/GSNetworkLibrary.h"
#include "Netcode/GSRollbackProxy.h"
#include "UI/GunsmithHUD.h"
#include "UI/GunsmithHUDWidget.h"
#include "VisualLogger/VisualLogger.h"

static bool PrintHealthChangesDebug = false;
static FAutoConsoleVariableRef CVarPrintHealthChangesDebug(
	TEXT("Gunsmith.Debug.PrintHitReactAnimationDebug"),
	PrintHealthChangesDebug,
	TEXT("Print out hit react details when an animation is played"));

static bool InvertMouseY = false;
static FAutoConsoleVariableRef CVarInvertMouseY(
	TEXT("Gunsmith.Input.InvertMouseY"),
	InvertMouseY,
	TEXT("Flips mouse Y controls"));

DEFINE_LOG_CATEGORY_STATIC(LogGunsmithMoverCharacter, Log, All);

// ReSharper disable CppDeclaratorNeverUsed
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithSamples_Weapon_Impact_Character, "Weapon.Impact.Character");

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithSamples_Weapon_Rifle, "Weapon.Tag.Rifle");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithSamples_Weapon_Rifle_Alternate, "Weapon.Tag.Rifle.Alternate");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithSamples_Weapon_Rifle_Premium, "Weapon.Tag.Rifle.Premium");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithSamples_Weapon_Pistol, "Weapon.Tag.Pistol");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithSamples_Weapon_Shotgun, "Weapon.Tag.Shotgun");

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithSamples_Weapon_Attachment_Scope, "Weapon.Attachment.Scope.Default");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithSamples_Weapon_Attachment_Silencer, "Weapon.Attachment.Silencer.Default");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithSamples_Weapon_Attachment_Magazine, "Weapon.Attachment.Magazine.Default");

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithSamples_Weapon_Projectile, "Weapon.Emitter.Projectile.Default");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithSamples_Weapon_Projectile_Small, "Weapon.Emitter.Projectile.Small");
// ReSharper restore CppDeclaratorNeverUsed

#if !UE_BUILD_SHIPPING
TArray<uint32> AGunsmithMoverCharacter::EnabledDebugMovers;
#endif

FAutoConsoleCommandWithWorldAndArgs FCmdGunsmithDebugMove
(
	TEXT("Gunsmith.Debug.MoveCharacter"),
	TEXT("Moves the local character for weapon accuracy testing"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& InParams, const UWorld* InWorld)
	{
		if(InParams.Num() == 0 || !InWorld)
		{
			return;
		}

		const bool bEnable = InParams[0].ToBool();
		if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(InWorld, 0))
		{
			const uint32 ControllerID = PlayerController->GetUniqueID();

#if !UE_BUILD_SHIPPING
			if (bEnable)
			{
				AGunsmithMoverCharacter::EnabledDebugMovers.AddUnique(ControllerID);
			}
			else
			{
				AGunsmithMoverCharacter::EnabledDebugMovers.Remove(ControllerID);
			}
			
			if (AGunsmithMoverCharacter* MoverCharacter = Cast<AGunsmithMoverCharacter>(PlayerController->GetPawn()))
			{
				MoverCharacter->EnableDebugMovement(bEnable);
			}
#endif
		}
	})
);

namespace GunsmithMoverCharacterNames
{
	static const FName CharacterMotionComponent = TEXT("MoverComponent");
	static const FName HealthComponent = TEXT("HealthComponent");
	static const FName RollbackComponent = TEXT("RollbackComponent");
};

AGunsmithMoverCharacter::AGunsmithMoverCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{		
	CharacterMotionComponent = CreateDefaultSubobject<UGSMoverComponent>(GunsmithMoverCharacterNames::CharacterMotionComponent);
	ensure(CharacterMotionComponent);

	HealthComponent = CreateDefaultSubobject<UGSHealthComponent>(GunsmithMoverCharacterNames::HealthComponent);
	HealthComponent->OnDamageTaken.AddDynamic(this, &AGunsmithMoverCharacter::OnDamageTaken);
	HealthComponent->OnDeath.AddDynamic(this, &AGunsmithMoverCharacter::OnDeath);

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 200.0f;
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	CameraComponent->bUsePawnControlRotation = true;

	RollbackComponent = CreateDefaultSubobject<UGSRollbackComponent>(GunsmithMoverCharacterNames::RollbackComponent);

	SetReplicatingMovement(false);	// disable Actor-level movement replication, since our Mover component will handle it

	PrimaryActorTick.bCanEverTick = true;
}

void AGunsmithMoverCharacter::BeginPlay()
{	
	Super::BeginPlay();

	RollbackComponent->OnPostSimulation.AddDynamic(this, &AGunsmithMoverCharacter::OnPostWorldSimulation);
	RollbackComponent->OnFinalizeFrame.AddDynamic(this, &AGunsmithMoverCharacter::OnPostFinalizeFrame);

	AGSRollbackProxy* RollbackProxy = RollbackComponent->GetRollbackProxy();
	if (RollbackProxy)
	{
		if (Mesh)
		{
			RollbackProxy->SetupTrackedMesh(Mesh, RollbackCollisionProfileName, RollbackCollisionObjectType);

			// Use the character capsule as our simple collision
			if (CapsuleComponent)
			{
				constexpr float SizeMultiplier = 1.4f;
				const FRotator CapsuleRelativeRotation = CapsuleComponent->GetRelativeRotation();
				const FVector CapsuleCenter = CapsuleRelativeRotation.Quaternion().GetUpVector() * CapsuleComponent->GetScaledCapsuleHalfHeight();
				RollbackProxy->SetupCapsuleComponent(NAME_None, CapsuleCenter, CapsuleRelativeRotation, CapsuleComponent->GetScaledCapsuleRadius() * SizeMultiplier, CapsuleComponent->GetScaledCapsuleHalfHeight() * SizeMultiplier, false);
			}

			// Extract bone info from the skeletal mesh to create the complex collision
			RollbackProxy->SetupSkeletalMeshRenderer(Mesh, RollbackBones);

			// Remove collision from projectiles but make sure we can still trace against it for weapon visibility
			Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
			Mesh->SetCollisionResponseToChannel(UGSDeveloperSettings::Get()->DefaultTargetTraceCollisionChannel, ECR_Block);

			if (UGSNetworkLibrary::IsServer(this))
			{
				// Make sure we update the anim on the server too
				Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
			}
		}
	}

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		PC->PlayerCameraManager->ViewPitchMax = 89.0f;
		PC->PlayerCameraManager->ViewPitchMin = -89.0f;

		if (AGunsmithHUD* HUD = Cast<AGunsmithHUD>(PC->GetHUD()))
		{
			if (UGunsmithHUDWidget* HUDWidget = HUD->GetHUDWidget())
			{
				HUDWidget->SetupForCharacter(this);
			}
		}
	}

#if !UE_BUILD_SHIPPING
	if (UGSWorldStateSubsystem* WorldStateSubsystem = GetWorld()->GetSubsystem<UGSWorldStateSubsystem>())
	{
		WorldStateSubsystem->RegisterProjectileCreatedDelegate(this).AddDynamic(this, &AGunsmithMoverCharacter::OnProjectileCreated);
	}
#endif

	SaveInitialActorRotation();
}

void AGunsmithMoverCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	const float FOVDelta = ((bIsFOVIncreasing ? 1.0f : -1.0f) * DeltaSeconds) / FOVChangeTime;
	const float NewFOVAlpha = FMath::Clamp(CurrentADSFOVAlpha + FOVDelta, 0.0f, 1.0f);

	if (NewFOVAlpha != CurrentADSFOVAlpha)
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			if (APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager)
			{
				if (NewFOVAlpha == 0.0f)
				{
					CameraManager->UnlockFOV();
				}
				else
				{
					const float FOVValue = FMath::Lerp(InitialFOVValue, ADSFOVValue, NewFOVAlpha);
					CameraManager->SetFOV(FOVValue);
				}

				CurrentADSFOVAlpha = NewFOVAlpha;
			}
		}
	}

#if !UE_BUILD_SHIPPING
	UpdateDebugMovement(DeltaSeconds);
#endif
}

void AGunsmithMoverCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveInputAction)
		{
			Input->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &AGunsmithMoverCharacter::OnMoveTriggered);
			Input->BindAction(MoveInputAction, ETriggerEvent::Completed, this, &AGunsmithMoverCharacter::OnMoveCompleted);
		}

		if (LookInputMouseAction)
		{
			Input->BindAction(LookInputMouseAction, ETriggerEvent::Triggered, this, &AGunsmithMoverCharacter::OnLookTriggered);
		}
		
		if (LookInputGamepadAction)
		{
			Input->BindAction(LookInputGamepadAction, ETriggerEvent::Triggered, this, &AGunsmithMoverCharacter::OnLookTriggered);
		}

		if (JumpInputAction)
		{
			Input->BindAction(JumpInputAction, ETriggerEvent::Started, this, &AGunsmithMoverCharacter::OnJumpStarted);
			Input->BindAction(JumpInputAction, ETriggerEvent::Completed, this, &AGunsmithMoverCharacter::OnJumpReleased);
		}

		if (FireAction)
		{
			Input->BindAction(FireAction, ETriggerEvent::Started, this, &AGunsmithMoverCharacter::BeginShooting);
			Input->BindAction(FireAction, ETriggerEvent::Completed, this, &AGunsmithMoverCharacter::EndShooting);
		}

		if (ADSAction)
		{
			Input->BindAction(ADSAction, ETriggerEvent::Started, this, &AGunsmithMoverCharacter::BeginADS);
			Input->BindAction(ADSAction, ETriggerEvent::Completed, this, &AGunsmithMoverCharacter::EndADS);
		}

		if (ReloadAction)
		{
			Input->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &AGunsmithMoverCharacter::OnReloadPressed);
		}

		for (int32 i = 0; i < EquipmentSlotActions.Num(); i++)
		{
			if (EquipmentSlotActions[i])
			{
				Input->BindAction(EquipmentSlotActions[i], ETriggerEvent::Triggered, this, &AGunsmithMoverCharacter::OnEquipmentSlotPressed, i);
			}
		}
	}
}

void AGunsmithMoverCharacter::BeginShooting()
{
	bIsShootingInputDown = true;
	bHasJustPressedShootingInput = true;
}

void AGunsmithMoverCharacter::EndShooting()
{
	bIsShootingInputDown = false;
}

void AGunsmithMoverCharacter::BeginADS()
{
	bIsADSInputDown = true;
}

void AGunsmithMoverCharacter::EndADS()
{
	bIsADSInputDown = false;
}

void AGunsmithMoverCharacter::ApplyLookInput(const FVector2D& LookInputs)
{
	LookRotation = ApplyRotation(LookRotation, LookInputs);

	SetControlRotation(GetAuthoritativeAimRotation() + CurrentRecoilRotation);
}

void AGunsmithMoverCharacter::OnProduceShootingInput(float DeltaMs, FGSShootingInputState& InputCmd)
{
	FGSDefaultShootingInputs& DefaultInputs = InputCmd.DataCollection.FindOrAddMutableDataByType<FGSDefaultShootingInputs>();

	if (!Controller || GetWorld()->IsNetMode(NM_DedicatedServer))
	{
		return;
	}
	
	DefaultInputs.bIsShooting = bIsShootingInputDown || bHasJustPressedShootingInput;

#if !UE_BUILD_SHIPPING
	DefaultInputs.bIsShooting |= ShootingComponent->GetAutoShootData().PlayerIndex != INDEX_NONE;
#endif
	
	bHasJustPressedShootingInput = false;

	DefaultInputs.bIsADSDown = bIsADSInputDown;

	DefaultInputs.bIsReloadTriggered = bIsReloadTriggered;
	bIsReloadTriggered = false;
	
	DefaultInputs.LookRotation = GetAuthoritativeAimRotation();

	const APlayerController* PlayerController = GetController<APlayerController>();
	if (PlayerController && PlayerController->PlayerCameraManager)
	{
		DefaultInputs.CameraLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
	}

	DefaultInputs.EquippedWeaponSlot = CurrentWeaponSlot;
}

void AGunsmithMoverCharacter::OnADSStateChanged(bool bADSEnabled)
{
	Super::OnADSStateChanged(bADSEnabled);

	bIsFOVIncreasing = bADSEnabled;
}

void AGunsmithMoverCharacter::SaveInitialActorRotation()
{
	// Initialize the simulation rotation to match the characters spawn rotation
	FRotator ActorRotation = GetActorRotation();
	SetControlRotation(ActorRotation);

	LookRotation = ActorRotation;
}

void AGunsmithMoverCharacter::NotifyRestarted()
{
	Super::NotifyRestarted();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			EnhancedInputSubsystem->ClearAllMappings();
			EnhancedInputSubsystem->AddMappingContext(InputMappingContext, 0);
		}
	}
}

void AGunsmithMoverCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	SaveInitialActorRotation();

#if !UE_BUILD_SHIPPING
	if (Controller && EnabledDebugMovers.Contains(Controller->GetUniqueID()))
	{
		EnableDebugMovement(true);
	}
#endif
}

FRotator AGunsmithMoverCharacter::GetBaseAimRotation() const
{
	if (IsLocallyControlled() && IsValid(ShootingComponent))
	{
		return LookRotation + CurrentRecoilRotation;		
	}
	
	return Super::GetBaseAimRotation();
}

FVector AGunsmithMoverCharacter::GetVelocity() const
{	
	return GetMoverComponent()->GetVelocity();
}

float AGunsmithMoverCharacter::GetDamageMultiplierForHitComponent_Implementation(const UPrimitiveComponent* Component) const
{
	if (AGSRollbackProxy* RollbackProxy = RollbackComponent->GetRollbackProxy())
	{
		FGSRollbackProxyColliderData ColliderData;
		if (RollbackProxy->GetColliderDataForComponent(Component, ColliderData))
		{
			if (BoneDamageMultipliers.Contains(ColliderData.BoneName))
			{
				return BoneDamageMultipliers[ColliderData.BoneName];
			}
		}
	}

	return 1.0f;
}

void AGunsmithMoverCharacter::ProduceInput_Implementation(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult)
{
	OnProduceMoverInput(static_cast<float>(SimTimeMs), InputCmdResult);
}

void AGunsmithMoverCharacter::OnProduceMoverInput(float DeltaMs, FMoverInputCmdContext& OutInputCmd)
{
	if (bIsDead)
	{
		return;
	}
	
	// Generate user commands. Called right before the Character movement simulation will tick (for a locally controlled pawn)
	// This isn't meant to be the best way of doing a camera system. It is just meant to show a couple of ways it may be done
	// and to make sure we can keep distinct the movement, rotation, and view angles.
	// Styles 1-3 are really meant to be used with a gamepad.
	//
	// Its worth calling out: the code that happens here is happening *outside* of the Character movement simulation. All we are doing
	// is generating the input being fed into that simulation. That said, this means that A) the code below does not run on the server
	// (and non controlling clients) and B) the code is not rerun during reconcile/resimulates. Use this information guide any
	// decisions about where something should go (such as aim assist, lock on targeting systems, etc): it is hard to give absolute
	// answers and will depend on the game and its specific needs. In general, at this time, I'd recommend aim assist and lock on 
	// targeting systems to happen /outside/ of the system, i.e, here. But I can think of scenarios where that may not be ideal too.

	FCharacterDefaultInputs& CharacterInputs = OutInputCmd.InputCollection.FindOrAddMutableDataByType<FCharacterDefaultInputs>();

	if (Controller == nullptr)
	{
		if (GetLocalRole() == ROLE_Authority && GetRemoteRole() == ROLE_SimulatedProxy)
		{
			static const FCharacterDefaultInputs DoNothingInput;
			// If we get here, that means this pawn is not currently possessed and we're choosing to provide default do-nothing input
			CharacterInputs = DoNothingInput;
		}

		// We don't have a local controller so we can't run the code below. This is ok. Simulated proxies will just use previous input when extrapolating
		return;
	}

	if (SpringArmComponent)
	{
		SpringArmComponent->bUsePawnControlRotation = true;
	}

	if (!UGSNetworkLibrary::IsServer(this))
	{
		CharacterInputs.ControlRotation = GetBaseAimRotation();
	}
	else
	{
		const APlayerController* PC = Cast<APlayerController>(Controller);
		if (PC)
		{
			CharacterInputs.ControlRotation = PC->GetControlRotation();
		}
	}

	// Favor velocity input 
	const bool bUsingInputIntentForMove = CachedMoveInputVelocity.IsZero();

	if (bUsingInputIntentForMove)
	{
		const FVector FinalDirectionalIntent = CharacterInputs.ControlRotation.RotateVector(CachedMoveInputIntent);
		CharacterInputs.SetMoveInput(EMoveInputType::DirectionalIntent, FinalDirectionalIntent);
	}
	else
	{
		CharacterInputs.SetMoveInput(EMoveInputType::Velocity, CachedMoveInputVelocity);
	}

#if !UE_BUILD_SHIPPING
	if (bIsDebugMoving)
	{
		const FVector MoveIntent = GetActorRightVector() * (bDebugMovingRight ? 1 : -1) * (1.0f - FMath::RandRange(0.01f, 0.02f));		
		CharacterInputs.SetMoveInput(EMoveInputType::DirectionalIntent, MoveIntent);
	}
#endif
	
	// Figure out intended orientation
	CharacterInputs.OrientationIntent = CharacterInputs.ControlRotation.Vector().GetSafeNormal2D();

	CharacterInputs.bIsJumpPressed = bIsJumpPressed || bIsDebugJumping;
	CharacterInputs.bIsJumpJustPressed = bIsJumpJustPressed || bIsDebugJumping;
	CharacterInputs.SuggestedMovementMode = NAME_None;

	// Convert inputs to be relative to the current movement base (depending on options and state)
	CharacterInputs.bUsingMovementBase = false;

	if (const UCharacterMoverComponent* MoverComp = GetComponentByClass<UCharacterMoverComponent>())
	{
		if (UPrimitiveComponent* MovementBase = MoverComp->GetMovementBase())
		{
			const FName MovementBaseBoneName = MoverComp->GetMovementBaseBoneName();

			FVector RelativeMoveInput, RelativeOrientDir;

			UBasedMovementUtils::TransformWorldDirectionToBased(MovementBase, MovementBaseBoneName, CharacterInputs.GetMoveInput(), RelativeMoveInput);
			UBasedMovementUtils::TransformWorldDirectionToBased(MovementBase, MovementBaseBoneName, CharacterInputs.OrientationIntent, RelativeOrientDir);

			CharacterInputs.SetMoveInput(CharacterInputs.GetMoveInputType(), RelativeMoveInput);
			CharacterInputs.OrientationIntent = RelativeOrientDir;

			CharacterInputs.bUsingMovementBase = true;
			CharacterInputs.MovementBase = MovementBase;
			CharacterInputs.MovementBaseBoneName = MovementBaseBoneName;
		}
	}

	// Clear/consume temporal movement inputs. We are not consuming others in the event that the game world is ticking at a lower rate than the Mover simulation. 
	// In that case, we want most input to carry over between simulation frames.
	{
		bIsJumpJustPressed = false;
		bIsDebugJumping = false;
	}
}

FRotator AGunsmithMoverCharacter::ApplyRotation(const FRotator& InLookRotation, const FVector2D& LookInputs) const
{
	FRotator OutLookRotation = InLookRotation;
	OutLookRotation.Yaw = InLookRotation.Yaw + LookInputs.X * InputConfig.LookInputSpeed.X;
	OutLookRotation.Pitch = FMath::Clamp(InLookRotation.Pitch + LookInputs.Y * InputConfig.LookInputSpeed.Y, InputConfig.InputPitchMin, InputConfig.InputPitchMax);
	return OutLookRotation;
}

void AGunsmithMoverCharacter::TriggerReload()
{
	bIsReloadTriggered = true;
}

void AGunsmithMoverCharacter::OnMoveTriggered(const FInputActionValue& Value)
{
	const FVector MovementVector = Value.Get<FVector>();
	CachedMoveInputIntent.X = FMath::Clamp(MovementVector.X, -1.0f, 1.0f);
	CachedMoveInputIntent.Y = FMath::Clamp(MovementVector.Y, -1.0f, 1.0f);
	CachedMoveInputIntent.Z = FMath::Clamp(MovementVector.Z, -1.0f, 1.0f);
}

void AGunsmithMoverCharacter::OnMoveCompleted(const FInputActionValue& Value)
{
	CachedMoveInputIntent = FVector::ZeroVector;
}

void AGunsmithMoverCharacter::OnLookTriggered(const FInputActionValue& Value)
{
	FVector2D LookVector = Value.Get<FVector2D>();

	if (InvertMouseY)
	{
		LookVector.Y *= -1;
	}
	
	ApplyLookInput(LookVector);
}

void AGunsmithMoverCharacter::OnJumpStarted(const FInputActionValue& Value)
{
	bIsJumpJustPressed = !bIsJumpPressed;
	bIsJumpPressed = true;
}

void AGunsmithMoverCharacter::OnJumpReleased(const FInputActionValue& Value)
{
	bIsJumpPressed = false;
	bIsJumpJustPressed = false;
}

void AGunsmithMoverCharacter::OnReloadPressed(const FInputActionValue& Value)
{
	TriggerReload();
}

void AGunsmithMoverCharacter::OnEquipmentSlotPressed(const FInputActionValue& Value, int32 Slot)
{
	if (ShootingComponent->HasEquippedWeapon(Slot))
	{
		CurrentWeaponSlot = Slot;	
	}
}

FRotator AGunsmithMoverCharacter::GetAuthoritativeAimRotation() const
{
#if !UE_BUILD_SHIPPING
	// Debug shoot at a character
	const FGunsmithAutoShootData& AutoShootData = ShootingComponent->GetAutoShootData();
	if (AutoShootData.PlayerIndex != INDEX_NONE)
	{
		APawn* TargetPawn = nullptr;

		// Find the target pawn
		if (AGameStateBase* GameState = GetWorld()->GetGameState())
		{
			for (APlayerState* TargetPlayerState : GameState->PlayerArray)
			{
				if (TargetPlayerState && TargetPlayerState->GetPlayerId() == AutoShootData.PlayerIndex)
				{
					TargetPawn = TargetPlayerState->GetPawn();
					break;
				}
			}
		}

		// Shoot from the camera location. This may need to change in some cases?
		APlayerController* PC = Cast<APlayerController>(Controller);
		if (TargetPawn && PC->PlayerCameraManager)
		{
			FVector TargetLocation = TargetPawn->GetActorLocation();
			TargetLocation.Z += TargetPawn->GetDefaultHalfHeight() / 2;
			
			if (AutoShootData.BoneName != NAME_None)
			{
				if (AGSCharacter* TargetCharacter = Cast<AGSCharacter>(TargetPawn))
				{
					if (USkeletalMeshComponent* SkeletalMeshComponent = TargetCharacter->GetMesh())
					{
						TargetLocation = SkeletalMeshComponent->GetBoneLocation(AutoShootData.BoneName);
					}
				}
			}

			const FRotator TargetRotation = (TargetLocation - PC->PlayerCameraManager->GetCameraLocation()).Rotation();
			return TargetRotation;
		}
	}
#endif

	return LookRotation;
}

void AGunsmithMoverCharacter::PlayHitReact(int32 Seed, const FVector& HitNormal) const
{
	const USkeletalMeshComponent* MeshToUse = GetMesh();

	if (!MeshToUse)
	{
		return;
	}
	
	UAnimInstance* AnimInstance = MeshToUse->GetAnimInstance();

	if (!AnimInstance)
	{
		return;
	}

	const TArray<TObjectPtr<UAnimMontage>>* HitReacts;
	const float ForwardDotProduct = GetActorForwardVector().Dot(HitNormal);
	const float RightDotProduct = GetActorRightVector().Dot(HitNormal);

	if (RightDotProduct >= -0.5f && RightDotProduct <= 0.5f)
	{
		if (ForwardDotProduct > 0.0f)
		{
			HitReacts = &HitReactsFront;
		}
		else
		{
			HitReacts = &HitReactsBack;
		}
	}
	else
	{
		if (RightDotProduct > 0.0f)
		{
			HitReacts = &HitReactsRight;
		}
		else
		{
			HitReacts = &HitReactsLeft;
		}
	}

	if (HitReacts && HitReacts->Num() > 0)
	{		
		FRandomStream NewRand = FRandomStream(Seed);
		const int32 RandomIndex = NewRand.RandRange(0, HitReacts->Num() - 1);
		TObjectPtr<UAnimMontage> RandomHitReact = (*HitReacts)[RandomIndex];

		if (PrintHealthChangesDebug)
		{
			if (UGSWorldStateSubsystem* WorldStateSubsystem = GetWorld()->GetSubsystem<UGSWorldStateSubsystem>())
			{
				FWorldContext* WorldContext = GEngine->GetWorldContextFromWorld(GetWorld());
				UE_LOG(LogGunsmithMoverCharacter, Log, TEXT("PIEInstance %d Playing hit react %d Frame %d Actual Frame %d"), WorldContext ? WorldContext->PIEInstance : -1, RandomIndex, Seed, WorldStateSubsystem->GetCurrentSimulationFrameIndex().ServerFrame);
			}
		}
		
		AnimInstance->Montage_Play(RandomHitReact);
	}
}

void AGunsmithMoverCharacter::DrawCurrentLocationDebug(bool bRoundToFullFrame, const FName& LogReference)
{
	UWorld* World = GetWorld();
	
	if (!World || !RollbackComponent)
	{
		return;
	}
	
	UGSWorldStateSubsystem* WorldStateSubsystem = World->GetSubsystem<UGSWorldStateSubsystem>();
	AGSRollbackProxy* RollbackProxy = RollbackComponent->GetRollbackProxy();

	if (!WorldStateSubsystem || !RollbackProxy)
	{
		return;
	}
	
	FGSFrameIndex CurrentFrameIndex = WorldStateSubsystem->GetCurrentSimulationFrameIndex();

	// Check if the actor is mid rollback. If so we are drawing the values at that frame instead of the active world frame
	const int32 CurrentRollbackFrame = RollbackComponent->GetCurrentRollbackFrame();
	if (CurrentRollbackFrame != INDEX_NONE)
	{
		CurrentFrameIndex.InterpolatedFrame = CurrentRollbackFrame;
		CurrentFrameIndex.InterpolatedRemainder = RollbackComponent->GetCurrentRollbackPercentage();
	}
	
	UGSWorldStateSubsystem::SetCustomVLogFrame(this, CurrentFrameIndex.InterpolatedFrame);
	
	FGSRollbackProxyFrameState WorldFrameState;
	FGSRollbackProxyFrameState FrameStateToUse;
	if (RollbackProxy->GetCurrentFrameState(WorldFrameState))
	{
		// If an interpolation percentage is provided we need to find the frame state at the start of the frame so that we can compare against the server frame as that only ever records in full frames
		int32 PreviousFrameIndex = -1;
		GSHistoryFrame* PreviousFrame = nullptr;
		if (bRoundToFullFrame && CurrentFrameIndex.InterpolatedRemainder != 0.0f)
		{
			for (UGSTrackedHistory* History : RollbackComponent->GetTrackedHistory())
			{
				if (UGSTrackedProxyHistory* ProxyHistory = Cast<UGSTrackedProxyHistory>(History))
				{
					PreviousFrameIndex = CurrentFrameIndex.InterpolatedFrame > History->LastFrame ? History->LastFrame : CurrentFrameIndex.InterpolatedFrame - 1;
					if (ProxyHistory->GetHistoryForFrame(PreviousFrameIndex, PreviousFrame))
					{
						GSRollbackProxyHistoryFrame* FromProxyFrame = static_cast<GSRollbackProxyHistoryFrame*>(PreviousFrame);
						const float PCT = FMath::GetMappedRangeValueClamped(FVector2D(PreviousFrameIndex + PreviousFrame->InterpolatedFramePercentage, CurrentFrameIndex.InterpolatedFrame + CurrentFrameIndex.InterpolatedRemainder), FVector2D(0.0f, 1.0f), CurrentFrameIndex.InterpolatedFrame);
						FrameStateToUse = AGSRollbackProxy::GetInterpolatedFrameState(FromProxyFrame->FrameState, WorldFrameState, PCT);
						break;
					}
				}
			}

			if (!PreviousFrame)
			{
				UE_LOG(LogGunsmithMoverCharacter, Error, TEXT("Unable to find history for frame %d. Cannot provide accurate VLOG data for %s"), PreviousFrameIndex, *GetName());
				UGSWorldStateSubsystem::ClearCustomVLogFrame(this);
				return;
			}
		}
		else
		{
			FrameStateToUse = WorldFrameState;
		}

		const float FinalInterpolationPCT = !bRoundToFullFrame ? CurrentFrameIndex.InterpolatedRemainder : 0.0f;
		constexpr bool bForceDebugString = true;
		TArray<UPrimitiveComponent*> HitComponents;
		RollbackProxy->DebugDraw(CurrentFrameIndex.InterpolatedFrame, FinalInterpolationPCT, EGSRollbackProxyCollisionState::Complex, this, FrameStateToUse, false, HitComponents, LogReference, bForceDebugString);
	}
	UGSWorldStateSubsystem::ClearCustomVLogFrame(this);
}

void AGunsmithMoverCharacter::OnDeath(UGSHealthComponent* AffectedHealthComponent, const FGSDamageRecord& DamageRecord, bool bIsPredicted)
{
	bIsDead = true;
	
	if (RagdollImpulseBone.IsValid() && Mesh)
	{
		if (DeathMontages.Num() > 0)
		{
			UAnimMontage* RandomMontage = DeathMontages[FMath::RandRange(0, DeathMontages.Num() - 1)];
			
			if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
			{			
				AnimInstance->Montage_Play(RandomMontage);
			}
		}
		
		Mesh->SetCollisionProfileName(TEXT("Ragdoll"));
		Mesh->SetAllBodiesBelowSimulatePhysics(RagdollImpulseBone, true);

		const FVector Velocity = GetVelocity();
		const FVector Impulse = Velocity + (Velocity.GetSafeNormal() * RagdollImpulseStrength);
		Mesh->AddImpulse(Impulse, RagdollImpulseBone, true);
	}

	if (DeathTimeBeforeDestroy > 0.0f)
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			Destroy();
		}), DeathTimeBeforeDestroy, false);
	}
	else
	{
		Destroy();
	}
}

void AGunsmithMoverCharacter::OnDamageTaken(UGSHealthComponent* AffectedHealthComponent, const FGSDamageRecord& DamageRecord, bool bIsPredicted)
{
	const bool bShouldPlayReact = GetLocalRole() == ROLE_AutonomousProxy && !bIsDead && DamageRecord.Damage > 5.0f;

	if (bShouldPlayReact)
	{
		const FVector HitDirection = GetActorLocation() - DamageRecord.HitSourceLocation;
		PlayHitReact(DamageRecord.Frame, HitDirection);	
	}
}

#if !UE_BUILD_SHIPPING
void AGunsmithMoverCharacter::EnableDebugMovement(bool bDebugMove)
{
	bIsDebugMoving = bDebugMove;

	TimeSinceDebugJump = FMath::RandRange(0.0f, TimeBetweenDebugJump);
}

void AGunsmithMoverCharacter::UpdateDebugMovement(float DeltaTime)
{
	if (bIsDebugMoving && !bIsDead)
	{
		TimeDebugMoving += DeltaTime;

		if (TimeDebugMoving > TimeToDebugMoveInOneDirection)
		{
			bDebugMovingRight = !bDebugMovingRight;
			TimeDebugMoving -= TimeToDebugMoveInOneDirection;
		}

		TimeSinceDebugJump += DeltaTime;

		if (TimeBetweenDebugJump > 0.0f && TimeSinceDebugJump > TimeBetweenDebugJump)
		{
			bIsDebugJumping = true;
			TimeSinceDebugJump = 0.0f;
		}
	}
}
#endif

void AGunsmithMoverCharacter::OnAutoShootProjectileHitTarget(int32 Frame, const FHitResult& Hit,
	UGSProjectileState* ProjectileState, const FGSProjectileFrameState& FrameState)
{
#if !UE_BUILD_SHIPPING
	if (AGSRollbackProxy* RollbackProxy = Cast<AGSRollbackProxy>(Hit.GetActor()))
	{
		if (APawn* HitPawn = Cast<APawn>(RollbackProxy->GetOwner()))
		{
			if (APlayerState* HitPlayer = HitPawn->GetPlayerState())
			{
				if (HitPlayer->GetPlayerId() == ShootingComponent->GetAutoShootData().PlayerIndex)
				{
					AutoShootProjectileHitCount++;
				}
			}
		}
	}
#endif
}

void AGunsmithMoverCharacter::OnAutoShootProjectileDestroyed(int32 Frame, bool bHitTarget, const FHitResult& HitResult,
	 UGSProjectileState* ProjectileState, const FGSProjectileFrameState& CurrentFrameData)
{
#if !UE_BUILD_SHIPPING
	UE_LOG(LogGunsmithTests, Log, TEXT("Auto shooting at target %d Hit %d out of %d Percentage %f"), ShootingComponent->GetAutoShootData().PlayerIndex, AutoShootProjectileHitCount, AutoShootProjectileCount, static_cast<float>(AutoShootProjectileHitCount) / FMath::Max(1, AutoShootProjectileCount));
#endif
}

void AGunsmithMoverCharacter::OnProjectileCreated(UGSProjectileState* ProjectileState)
{
#if !UE_BUILD_SHIPPING
	if (IsLocallyControlled() && ShootingComponent && ShootingComponent->GetAutoShootData().PlayerIndex != INDEX_NONE)
	{
		AutoShootProjectileCount++;

		ProjectileState->OnProjectileHitTarget.AddDynamic(this, &AGunsmithMoverCharacter::OnAutoShootProjectileHitTarget);
		ProjectileState->OnProjectileDestroyed.AddDynamic(this, &AGunsmithMoverCharacter::OnAutoShootProjectileDestroyed);
	}
#endif
}
