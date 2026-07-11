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
#include "GSLog.h"
#include "Netcode/GSRollbackComponent.h"
#include "InputAction.h"
#include "Weapon/GSShootingComponent.h"
#include "World/GSWorldStateSubsystem.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Character/GSMoverComponent.h"
#include "Character/GSMoverDeathMode.h"
#include "Character/GSSkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Game/GunsmithExampleWeaponDataAsset.h"
#include "Character/GSCharacterAnimationData.h"
#include "Engine/GameInstance.h"
#include "Game/GunsmithGameState.h"
#include "Game/GunsmithPlayerController.h"
#include "Game/GunsmithShootingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Health/GSHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Netcode/GSNetworkLibrary.h"
#include "Sound/SoundBase.h"
#include "UI/GunsmithHUD.h"
#include "UI/GunsmithHUDWidget.h"
#include "VisualLogger/VisualLogger.h"
#include "Weapon/GSWeaponsSubsystem.h"
#include "Weapon/GSWeaponTypeTags.h"
#include "Weapon/Emitter/GSWeaponEmitter.h"
#include "Weapon/Emitter/Output/Projectile/GSProjectileDataAsset.h"

static bool InvertMouseY = false;
static FAutoConsoleVariableRef CVarInvertMouseY(
	TEXT("Gunsmith.Input.InvertMouseY"),
	InvertMouseY,
	TEXT("Flips mouse Y controls"));

// ReSharper disable CppDeclaratorNeverUsed
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithExamples_ShootingComponent_Grenade, "Weapon.ShootingComponent.Grenade");

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithExamples_Weapon_Rifle, "Weapon.Tag.Rifle");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithExamples_Weapon_Rifle_Alternate, "Weapon.Tag.Rifle.Alternate");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithExamples_Weapon_Rifle_Premium, "Weapon.Tag.Rifle.Premium");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithExamples_Weapon_Pistol, "Weapon.Tag.Pistol");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithExamples_Weapon_Shotgun, "Weapon.Tag.Shotgun");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithExamples_Weapon_BeamRifle, "Weapon.Tag.BeamRifle");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithExamples_Weapon_RocketLauncher, "Weapon.Tag.RocketLauncher");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithExamples_Weapon_Grenade, "Weapon.Tag.Grenade");

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithExamples_Weapon_Attachment_Scope, "Weapon.Attachment.Scope.Default");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithExamples_Weapon_Attachment_Silencer, "Weapon.Attachment.Silencer.Default");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithExamples_Weapon_Attachment_Magazine, "Weapon.Attachment.Magazine.Default");

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithExamples_Weapon_Projectile, "Weapon.Emitter.Projectile.Default");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithExamples_Weapon_Projectile_Small, "Weapon.Emitter.Projectile.Small");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithExamples_Weapon_Projectile_Rocket, "Weapon.Emitter.Projectile.Rocket");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GunsmithExamples_Weapon_Projectile_Grenade, "Weapon.Emitter.Projectile.Grenade");
// ReSharper restore CppDeclaratorNeverUsed

#if !UE_BUILD_SHIPPING
TArray<TTuple<uint32, int32>> AGunsmithMoverCharacter::EnabledDebugMovers;
#endif

static FAutoConsoleCommandWithWorldAndArgs FCmdGunsmithDebugMove
(
	TEXT("Gunsmith.Debug.MoveCharacter"),
	TEXT("Moves the local character for weapon accuracy testing"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& InParams, const UWorld* InWorld)
	{
		if(InParams.Num() == 0 || !InWorld)
		{
			return;
		}

		const int32 MoveType = FCString::Atoi(*InParams[0]);
		const bool bEnable = MoveType > 0;
		if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(InWorld, 0))
		{
			const uint32 ControllerID = PlayerController->GetUniqueID();

#if !UE_BUILD_SHIPPING
			if (bEnable)
			{
				bool bFound = false;
				for (int32 i = 0; i < AGunsmithMoverCharacter::EnabledDebugMovers.Num(); i++)
				{
					TTuple<uint32, int32>& ExistingMover = AGunsmithMoverCharacter::EnabledDebugMovers[i];

					if (ExistingMover.Key == ControllerID)
					{
						ExistingMover.Value = MoveType;
						bFound = true;
						break;
					}
				}

				if (!bFound)
				{
					AGunsmithMoverCharacter::EnabledDebugMovers.Add({ ControllerID, MoveType });
				}
			}
			else
			{
				for (int32 i = 0; i < AGunsmithMoverCharacter::EnabledDebugMovers.Num(); i++)
				{
					const TTuple<uint32, int32>& Controller = AGunsmithMoverCharacter::EnabledDebugMovers[i];

					if (Controller.Key == ControllerID)
					{
						AGunsmithMoverCharacter::EnabledDebugMovers.RemoveAt(i--);	
					}
				}
			}
			
			if (AGunsmithMoverCharacter* MoverCharacter = Cast<AGunsmithMoverCharacter>(PlayerController->GetPawn()))
			{
				MoverCharacter->EnableDebugMovement(MoveType);
			}
#endif
		}
	})
);

static FAutoConsoleCommandWithWorldAndArgs FCmdGunsmithDebugEquip
(
	TEXT("Gunsmith.Debug.EquipCharacter"),
	TEXT("Equips the local character with equip data that matches the tag. The weapon must be preloaded via the GSWeaponsSubsystem"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& InParams, const UWorld* InWorld)
	{
		if(InParams.Num() == 0 || !InWorld)
		{
			return;
		}
		
		UGSWeaponsSubsystem* WeaponsSubsystem = InWorld->GetGameInstance()->GetSubsystem<UGSWeaponsSubsystem>();
		
		if (!WeaponsSubsystem)
		{
			return;
		}
		
		FGameplayTag EquipTag = FGameplayTag::RequestGameplayTag(FName(InParams[0]));
		
		if (!EquipTag.IsValid())
		{
			UE_LOG(LogGunsmith, Warning, TEXT("Unable to find gameplay tag %s"), *InParams[0])
			return;
		}
		
		UGSWeaponDataAsset* WeaponData = WeaponsSubsystem->GetLoadedWeaponData(EquipTag);
		
		if (!WeaponData)
		{
			UE_LOG(LogGunsmith, Warning, TEXT("Unable to find loaded weapon with tag %s"), *InParams[0])
			return;
		}
		
		if (AGunsmithPlayerController* LocalCharacter = Cast<AGunsmithPlayerController>(UGameplayStatics::GetPlayerController(InWorld, 0)))
		{
			LocalCharacter->Server_EquipWeapon(WeaponData);
		}
	})
);

namespace GunsmithMoverCharacterNames
{
	static const FName CharacterMotionComponent = TEXT("MoverComponent");
	static const FName LeftHandSocket = TEXT("weapon_l");
	static const FName GrenadeComponentName = TEXT("GrenadeComponent");
};

AGunsmithMoverCharacter::AGunsmithMoverCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass(GSCharacterNames::ShootingComponentName, UGunsmithShootingComponent::StaticClass()))
{		
	CharacterMotionComponent = CreateDefaultSubobject<UGSMoverComponent>(GunsmithMoverCharacterNames::CharacterMotionComponent);
	ensure(CharacterMotionComponent);

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 200.0f;
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	CameraComponent->bUsePawnControlRotation = true;
	
	GrenadeComponent = CreateDefaultSubobject<UGSShootingComponent>(GunsmithMoverCharacterNames::GrenadeComponentName);

	SetReplicatingMovement(false);	// disable Actor-level movement replication, since our Mover component will handle it

	PrimaryActorTick.bCanEverTick = true;
}

void AGunsmithMoverCharacter::BeginPlay()
{
	GrenadeComponent->SetInputProducer(this);
	GrenadeComponent->SetShouldCreateCrosshair(false);
	
	GrenadeComponent->OnEmitterEvent.AddUniqueDynamic(this, &AGunsmithMoverCharacter::OnGrenadeEvent);
	
	Super::BeginPlay();
	
	// Setup optional debug history for the gameplay debugger
	if (UGSAnimationDebugHistory* DebugHistory = RollbackComponent->RegisterDebugHistory<UGSAnimationDebugHistory, GSAnimationDebugHistoryFrame>())
	{
		DebugHistory->SkeletalMeshComponent = GetMesh();
	}
	
	if (UGSShootingComponentDebugHistory* DebugHistory = RollbackComponent->RegisterDebugHistory<UGSShootingComponentDebugHistory, GSShootingComponentDebugHistoryFrame>())
	{
		DebugHistory->ShootingComponent = ShootingComponent;
	}

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		SetupForPlayerController(PC);
	}
	
	if (UGSWorldStateSubsystem* WorldStateSubsystem = GetWorld()->GetSubsystem<UGSWorldStateSubsystem>())
	{		
#if !UE_BUILD_SHIPPING
		WorldStateSubsystem->RegisterProjectileCreatedDelegate(this).AddDynamic(this, &AGunsmithMoverCharacter::OnProjectileCreated);
#endif
	}

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
			Input->BindAction(ReloadAction, ETriggerEvent::Started, this, &AGunsmithMoverCharacter::OnReloadPressed);
		}

		if (NextWeaponAction)
		{
			Input->BindAction(NextWeaponAction, ETriggerEvent::Started, this, &AGunsmithMoverCharacter::OnNextWeaponPressed);
		}

		if (PreviousWeaponAction)
		{
			Input->BindAction(PreviousWeaponAction, ETriggerEvent::Started, this, &AGunsmithMoverCharacter::OnNextWeaponPressed);
		}
		
		if (GrenadeAction)
		{
			Input->BindAction(GrenadeAction, ETriggerEvent::Started, this, &AGunsmithMoverCharacter::OnGrenadeStarted);
			Input->BindAction(GrenadeAction, ETriggerEvent::Completed, this, &AGunsmithMoverCharacter::OnGrenadeReleased);
		}

		for (int32 i = 0; i < EquipmentSlotActions.Num(); i++)
		{
			if (EquipmentSlotActions[i])
			{
				Input->BindAction(EquipmentSlotActions[i], ETriggerEvent::Started, this, &AGunsmithMoverCharacter::OnEquipmentSlotPressed, i);
			}
		}
	}
}

void AGunsmithMoverCharacter::ServerStartDebugMovement(int32 MoveType)
{
	ServerRequestedDebugMove = MoveType;
	MARK_PROPERTY_DIRTY_FROM_NAME(AGunsmithMoverCharacter, ServerRequestedDebugMove, this);
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

void AGunsmithMoverCharacter::ForceDeath()
{
	OnDeath(nullptr, FGSDamageRecord(), false);
}

void AGunsmithMoverCharacter::SetupForPlayerController(APlayerController* PC)
{
	if (!ensure(PC))
	{
		return;
	}
	
	PC->PlayerCameraManager->ViewPitchMax = 89.0f;
	PC->PlayerCameraManager->ViewPitchMin = -89.0f;

	if (AGunsmithHUD* HUD = Cast<AGunsmithHUD>(PC->GetHUD()))
	{
		if (UGunsmithHUDWidget* HUDWidget = HUD->GetHUDWidget())
		{
			HUDWidget->SetupForCharacter(this);
		}
	}

	bHasInitializedController = true;
}

void AGunsmithMoverCharacter::OnADSStateChanged_Implementation(bool bADSEnabled)
{
	Super::OnADSStateChanged_Implementation(bADSEnabled);

	bIsFOVIncreasing = bADSEnabled;

	if (USoundBase* SoundToPlay = bADSEnabled ? StartADSSound : EndADSSound)
	{
		UGameplayStatics::PlaySound2D(this, SoundToPlay);
	}
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
			EnhancedInputSubsystem->AddMappingContext(InputMappingContext, 0);
		}
	}
}

void AGunsmithMoverCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!bHasInitializedController)
	{
		if (APlayerController* PC = Cast<APlayerController>(NewController))
		{
			SetupForPlayerController(PC);	
		}
	}
}

void AGunsmithMoverCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	SaveInitialActorRotation();

	if (!bHasInitializedController)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		SetupForPlayerController(PC);
	}

#if !UE_BUILD_SHIPPING
	if (Controller)
	{
		int32 ControllerID = Controller->GetUniqueID();
		
		if (MoveTypeToAddToDebugMove > 0)
		{
			EnabledDebugMovers.Emplace(TTuple<uint32, int32>(ControllerID, MoveTypeToAddToDebugMove));
			MoveTypeToAddToDebugMove = 0;
		}

		for (const TTuple<uint32, int32>& EnabledMover : EnabledDebugMovers)
		{
			if (EnabledMover.Key == ControllerID)
			{
				EnableDebugMovement(EnabledMover.Value);
			}
		}
	}
#endif
}

FRotator AGunsmithMoverCharacter::GetBaseAimRotation() const
{
#if !UE_BUILD_SHIPPING
	const FGunsmithAutoShootData& AutoShootData = ShootingComponent->GetAutoShootData();
	if (AutoShootData.PlayerIndex != INDEX_NONE)
	{
		return GetAuthoritativeAimRotation();
	}
#endif
	
	if (IsLocallyControlled() && IsValid(ShootingComponent))
	{
		return LookRotation + CurrentRecoilRotation;		
	}
	
	return Super::GetBaseAimRotation();
}

void AGunsmithMoverCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ServerRequestedDebugMove, Params);
}

void AGunsmithMoverCharacter::OnShootingMontageEnded()
{
	Super::OnShootingMontageEnded();
	
	bIsGrenadeAnimActive = false;
}

void AGunsmithMoverCharacter::OnWeaponEvent_Implementation(int32 EventType, const FGSEquipData& EquippedWeapon,
	UObject* SpawnedObject, bool bIsMainEmitterInstance)
{
	// Prevent shooting anim if grenade anim is active
	if (bIsMainEmitterInstance && bIsGrenadeAnimActive && (EventType == static_cast<int32>(EGSWeaponEmitterEventType::Trigger) || EventType == static_cast<int32>(EGSWeaponEmitterEventType::Hold)))
	{
		return;
	}
	
	Super::OnWeaponEvent_Implementation(EventType, EquippedWeapon, SpawnedObject, bIsMainEmitterInstance);
}

UGSShootingComponent* AGunsmithMoverCharacter::GetShootingComponent_Implementation(const FGameplayTag Tag) const
{
	if (Tag == TAG_GunsmithExamples_ShootingComponent_Grenade)
	{
		return GrenadeComponent;
	}
	
	return Super::GetShootingComponent_Implementation(Tag);
}

FName AGunsmithMoverCharacter::GetWeaponAttachmentSocketName_Implementation(
	const UGSWeaponDataAsset* WeaponDataAsset) const
{
	if (WeaponDataAsset->WeaponType == TAG_Weapon_Type_Grenade)
	{
		return GunsmithMoverCharacterNames::LeftHandSocket;
	}
	
	return Super::GetWeaponAttachmentSocketName_Implementation(WeaponDataAsset);
}

UGSCharacterAnimationData* AGunsmithMoverCharacter::GetAnimDataForWeaponType_Implementation(
	const UGSWeaponDataAsset* Weapon)
{
	if (const UGunsmithExampleWeaponDataAsset* ExampleAsset = Cast<UGunsmithExampleWeaponDataAsset>(Weapon))
	{
		return ExampleAsset->AnimationData.GetDefaultObject();
	}

	return nullptr;
}

FVector AGunsmithMoverCharacter::GetVelocity() const
{	
	return GetMoverComponent()->GetVelocity();
}

void AGunsmithMoverCharacter::ProduceInput_Implementation(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult)
{
	OnProduceMoverInput(static_cast<float>(SimTimeMs), InputCmdResult);
}

void AGunsmithMoverCharacter::OnProduceMoverInput(float DeltaMs, FMoverInputCmdContext& OutInputCmd)
{
	if (HealthComponent->IsDead())
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
	if (DebugMoveType > 0)
	{
		const FVector MoveIntent = GetActorRightVector() * (bDebugMovingRight ? 1 : -1) * (1.0f - FMath::RandRange(0.01f, 0.02f));		
		CharacterInputs.SetMoveInput(EMoveInputType::DirectionalIntent, MoveIntent);
	}
#endif
	
	// Figure out intended orientation
	CharacterInputs.OrientationIntent = CharacterInputs.ControlRotation.Vector().GetSafeNormal2D();

	CharacterInputs.bIsJumpPressed = bIsJumpPressed;
	CharacterInputs.bIsJumpJustPressed = bIsJumpJustPressed;
	CharacterInputs.SuggestedMovementMode = NAME_None;

#if !UE_BUILD_SHIPPING
	CharacterInputs.bIsJumpPressed |= bIsDebugJumping;
	CharacterInputs.bIsJumpJustPressed |= bIsDebugJumping;
#endif

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

#if !UE_BUILD_SHIPPING
		bIsDebugJumping = false;
#endif
	}
}

void AGunsmithMoverCharacter::OnProduceShootingInput(UGSShootingComponent* TargetShootingComponent, float DeltaMs,
	FGSShootingInputState& InputCmd)
{
	FGSDefaultShootingInputs& DefaultInputs = InputCmd.DataCollection.FindOrAddMutableDataByType<FGSDefaultShootingInputs>();

	if (!Controller || GetWorld()->IsNetMode(NM_DedicatedServer))
	{
		return;
	}
	
	const bool bIsGrenadeActive = bIsGrenadeInputDown || bIsGrenadeAnimActive;
	
	// Main shooting component
	if (TargetShootingComponent == ShootingComponent)
	{
		DefaultInputs.bIsShooting = !bIsGrenadeActive && (bIsShootingInputDown || bHasJustPressedShootingInput);
		bHasJustPressedShootingInput = false;

#if !UE_BUILD_SHIPPING
		DefaultInputs.bIsShooting |= ShootingComponent->GetAutoShootData().PlayerIndex != INDEX_NONE;
#endif
		
		DefaultInputs.bIsADSDown = !bIsGrenadeActive && bIsADSInputDown;
		
		DefaultInputs.bIsReloadTriggered = !bIsGrenadeActive && bIsReloadTriggered;
		bIsReloadTriggered = false;
		
		DefaultInputs.EquippedWeaponSlot = CurrentWeaponSlot;
	}
	// Grenade component
	else if (TargetShootingComponent == GrenadeComponent)
	{
		DefaultInputs.bIsShooting = bIsGrenadeInputDown || bHasJustPressedGrenadeInput;
		bHasJustPressedGrenadeInput = false;
	}
}

void AGunsmithMoverCharacter::OnProduceSharedShootingInput(float DeltaMs, FGSShootingInputState& InputCmd)
{
	FGSDefaultCameraInputs& DefaultInputs = InputCmd.DataCollection.FindOrAddMutableDataByType<FGSDefaultCameraInputs>();

	if (!Controller || GetWorld()->IsNetMode(NM_DedicatedServer))
	{
		return;
	}
	
	DefaultInputs.LookRotation = GetAuthoritativeAimRotation();

	const APlayerController* PlayerController = GetController<APlayerController>();
	if (PlayerController && PlayerController->PlayerCameraManager)
	{
		DefaultInputs.CameraLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
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

void AGunsmithMoverCharacter::OnNextWeaponPressed(const FInputActionValue& Value)
{
	ChangeWeapon(1);
}

void AGunsmithMoverCharacter::OnPreviousWeaponPressed(const FInputActionValue& Value)
{
	ChangeWeapon(-1);
}

void AGunsmithMoverCharacter::OnEquipmentSlotPressed(const FInputActionValue& Value, int32 Slot)
{
	if (ShootingComponent->HasEquippedWeapon(Slot))
	{
		CurrentWeaponSlot = Slot;	
	}
}

void AGunsmithMoverCharacter::OnGrenadeStarted(const FInputActionValue& Value)
{
	bIsGrenadeInputDown = true;
	bHasJustPressedGrenadeInput = true;
}

void AGunsmithMoverCharacter::OnGrenadeReleased(const FInputActionValue& Value)
{
	bIsGrenadeInputDown = false;
}

void AGunsmithMoverCharacter::ChangeWeapon(int32 Direction)
{
	if (ShootingComponent)
	{
		const int32 ActiveWeaponSlot = ShootingComponent->GetActiveWeaponSlot();
		int32 NextWeaponSlot = ActiveWeaponSlot + Direction;

		const int32 MaxSlots = ShootingComponent->GetNumMaxWeaponSlots();
		if (NextWeaponSlot < 0)
		{
			NextWeaponSlot = MaxSlots - 1;
		}
		else if (NextWeaponSlot >= MaxSlots)
		{
			NextWeaponSlot = 0;
		}

		CurrentWeaponSlot = NextWeaponSlot;
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

		// Try to find projectile speed
		float ProjectileSpeed = TNumericLimits<float>::Max();
		const TArray<UGSWeaponEmitter*> Emitters = ShootingComponent->GetEmitters();
		if (Emitters.Num() > 0)
		{
			if (UGSWeaponEmitter* MainEmitter = Emitters[0])
			{
				if (UGSProjectileDataAsset* ProjectileDataAsset = Cast<UGSProjectileDataAsset>(MainEmitter->GetAssociatedEmitterDataAsset()))
				{
					ProjectileSpeed = ProjectileDataAsset->GetTravelSpeed();
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

			// Aim to the targets future location
			const FVector TraceStartLocation = PC->PlayerCameraManager->GetCameraLocation();

			const float DistanceToTarget = (TargetLocation - TraceStartLocation).Size();
			const float FramesToHitTarget = DistanceToTarget / ProjectileSpeed;

			const FVector PawnVelocity = TargetPawn->GetVelocity();
			const FVector PredictionLocation = TargetLocation + PawnVelocity * FramesToHitTarget;

			const FRotator TargetRotation = (PredictionLocation - TraceStartLocation).Rotation();
			return TargetRotation;
		}
	}
#endif

	return LookRotation;
}

void AGunsmithMoverCharacter::OnDeath(UGSHealthComponent* AffectedHealthComponent, const FGSDamageRecord& DamageRecord, bool bIsPredicted)
{
	ShootingComponent->SetIsShootingDisabled(true);
	
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

		// Delay ragdoll before applying
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, [this]()
		{
			Mesh->SetCollisionProfileName(TEXT("Ragdoll"));
			Mesh->SetAllBodiesBelowSimulatePhysics(RagdollImpulseBone, true);

			const FVector Velocity = GetVelocity();
			const FVector Impulse = Velocity + (Velocity.GetSafeNormal() * RagdollImpulseStrength);
			Mesh->AddImpulse(Impulse, RagdollImpulseBone, true);
		}, FMath::RandRange(0.1f, 0.6f), false);
	}

	if (DeathTimeBeforeDestroy > 0.0f)
	{
		TWeakObjectPtr<AGunsmithMoverCharacter> WeakThis(this);
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
		{
			if (WeakThis.Get())
			{
				WeakThis->Destroy();
			}
		}), DeathTimeBeforeDestroy, false);
	}
	else
	{
		Destroy();
	}
}

#if !UE_BUILD_SHIPPING
void AGunsmithMoverCharacter::EnableDebugMovement(int32 MoveType)
{
	DebugMoveType = MoveType;

	TimeSinceDebugJump = FMath::RandRange(0.0f, TimeBetweenDebugJump);
}

void AGunsmithMoverCharacter::UpdateDebugMovement(float DeltaTime)
{
	if (DebugMoveType > 0 && !HealthComponent->IsDead())
	{
		TimeDebugMoving += DeltaTime;

		if (TimeDebugMoving > TimeToDebugMoveInOneDirection)
		{
			bDebugMovingRight = !bDebugMovingRight;
			TimeDebugMoving -= TimeToDebugMoveInOneDirection;
		}

		TimeSinceDebugJump += DeltaTime;

		if (DebugMoveType == 2 && TimeBetweenDebugJump > 0.0f && TimeSinceDebugJump > TimeBetweenDebugJump)
		{
			bIsDebugJumping = true;
			TimeSinceDebugJump = 0.0f;
		}
	}
}
#endif

void AGunsmithMoverCharacter::OnAutoShootProjectileHitTarget(int32 Frame, const FHitResult& Hit,
	const UGSProjectileState* ProjectileState, const FGSProjectileFrameState& FrameState)
{
#if !UE_BUILD_SHIPPING
	if (APawn* HitPawn = Cast<APawn>(Hit.GetActor()))
	{
		if (APlayerState* HitPlayer = HitPawn->GetPlayerState())
		{
			if (HitPlayer->GetPlayerId() == ShootingComponent->GetAutoShootData().PlayerIndex)
			{
				AutoShootProjectileHitCount++;
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

void AGunsmithMoverCharacter::OnGrenadeEvent(int32 EventType, const FGSEquipData& EquippedWeapon,
	UObject* SpawnedObject, bool bIsMainEmitterInstance)
{
	if (bIsMainEmitterInstance && (EventType == static_cast<int32>(EGSWeaponEmitterEventType::Trigger) || EventType == static_cast<int32>(EGSWeaponEmitterEventType::Hold)))
	{
		PlayShootingAnimation(GrenadeComponent, true);
		bIsGrenadeAnimActive = true;
	}
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

void AGunsmithMoverCharacter::OnRep_ServerRequestedDebugMove()
{
#if !UE_BUILD_SHIPPING
	if (Controller)
	{
		int32 ControllerID = Controller->GetUniqueID();
		if (ServerRequestedDebugMove > 0)
		{
			EnabledDebugMovers.AddUnique(TTuple<uint32, int32>(ControllerID, ServerRequestedDebugMove));
		}
		else
		{
			for (int32 i = 0; i < EnabledDebugMovers.Num(); i++)
			{
				const TTuple<uint32, int32>& Mover = EnabledDebugMovers[i];

				if (Mover.Key == ControllerID)
				{
					EnabledDebugMovers.RemoveAt(i--);	
				}
			}
		}
	}
	else
	{
		MoveTypeToAddToDebugMove = true;
	}
			
	EnableDebugMovement(ServerRequestedDebugMove);
#endif
}
