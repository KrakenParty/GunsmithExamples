// Copyright Kraken Party Limited. All Rights Reserved.


#include "Game/GunsmithPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GSGameplayLibrary.h"
#include "GunsmithBlueprintFunctionLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Game/GunsmithSpectatorPawn.h"
#include "Game/Effects/NumberPops/LyraNumberPopComponent_NiagaraText.h"
#include "GameFramework/InputDeviceSubsystem.h"
#include "GameFramework/Pawn.h"
#include "Health/GSHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Netcode/GSNetworkLibrary.h"
#include "UI/GunsmithHUD.h"
#include "Weapon/GSEquipData.h"
#include "Weapon/GSShootingComponent.h"

static FAutoConsoleCommandWithWorld FCmdGunsmithKillPlayer
(
	TEXT("Gunsmith.KillPlayer"),
	TEXT("Kills the pawn controlled by the player who executes the function"),
	FConsoleCommandWithWorldDelegate::CreateLambda([](const UWorld* InWorld)
	{
		if (AGunsmithPlayerController* PlayerController = Cast<AGunsmithPlayerController>(UGameplayStatics::GetPlayerController(InWorld, 0)))
		{
			PlayerController->Server_KillPawn();
		}
	}),
	ECVF_Cheat
);

AGunsmithPlayerController::AGunsmithPlayerController()
{
	NumberPopComponent = CreateDefaultSubobject<ULyraNumberPopComponent_NiagaraText>("LyraNumberPopComponent");
}

void AGunsmithPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Set the default input mode
	FInputModeGameOnly GameMode;
	SetInputMode(GameMode);
	
	if (UInputDeviceSubsystem* InputDeviceSubsystem = GEngine->GetEngineSubsystem<UInputDeviceSubsystem>())
	{
		InputDeviceSubsystem->OnInputHardwareDeviceChanged.AddDynamic(this, &AGunsmithPlayerController::OnHardwareDeviceChanged);
		OnHardwareDeviceChanged(GetPlatformUserId(), FInputDeviceId());
	}
}

void AGunsmithPlayerController::OnRep_Pawn()
{
	ClientRetryClientRestart(GetPawn());
	
	Super::OnRep_Pawn();

	UpdateSpectatorState();
}

void AGunsmithPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	if (UGSNetworkLibrary::HasAuthority(this))
	{
		UpdateSpectatorState();
	}
}

void AGunsmithPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (CommonInputs)
	{
		UGunsmithCommonInputs* CommonInputObject = CommonInputs->GetDefaultObject<UGunsmithCommonInputs>();
		if (UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (TObjectPtr<UInputMappingContext> MappingContext : CommonInputObject->AdditionalInputMappingContexts)
			{
				EnhancedInputSubsystem->AddMappingContext(MappingContext, 0);
			}
		}
		
		if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent))
		{
			if (CommonInputObject->PauseInputAction)
			{
				Input->BindAction(CommonInputObject->PauseInputAction, ETriggerEvent::Started, this, &AGunsmithPlayerController::OnPausePressed);
			}

			if (CommonInputObject->CancelUIInputAction)
			{
				Input->BindAction(CommonInputObject->CancelUIInputAction, ETriggerEvent::Started, this, &AGunsmithPlayerController::OnCancelPressed);
			}
		}
	}
}

bool AGunsmithPlayerController::ShouldShowMouseCursor() const
{
	if (AGunsmithHUD* GunsmithHUD = GetHUD<AGunsmithHUD>())
	{
		return GunsmithHUD->HasAnyActiveWidgets() && !bWasLastUsingGamepad;
	}

	return false;
}

void AGunsmithPlayerController::SetInitialLocationAndRotation(const FVector& NewLocation, const FRotator& NewRotation)
{
	Super::SetInitialLocationAndRotation(NewLocation, NewRotation);

	if (UGSNetworkLibrary::IsServer(this))
	{
		COMPARE_ASSIGN_AND_MARK_PROPERTY_DIRTY(AGunsmithPlayerController, SpawnRotation, GetControlRotation(), this);
	}
}

void AGunsmithPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	Params.Condition = COND_OwnerOnly;
	DOREPLIFETIME_WITH_PARAMS_FAST(AGunsmithPlayerController, SpawnRotation, Params);
}

ASpectatorPawn* AGunsmithPlayerController::SpawnSpectatorPawn()
{
	// Server is spawned with the correct rotation, client never receives this as ControlRotation is not replicated
	if (GetWorld()->IsNetMode(NM_Client))
	{
		SetControlRotation(SpawnRotation);
	}
	
	return Super::SpawnSpectatorPawn();
}

void AGunsmithPlayerController::SetPaused(bool bPaused)
{	
	if (AGunsmithHUD* GunsmithHUD = GetHUD<AGunsmithHUD>())
	{
		GunsmithHUD->SetPaused(!GunsmithHUD->HasAnyActiveWidgets());
	}
}

void AGunsmithPlayerController::SetUIInputMode(bool bEnabled, UWidget* WidgetToFocus)
{
	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		const FGameplayTag& NewMode = bEnabled ? TAG_InputMode_UI : UE::EnhancedInput::InputMode_Default;
		InputSubsystem->SetInputMode(FGameplayTagContainer(NewMode));
	}
	
	if (bEnabled)
	{
		FInputModeUIOnly UIMode;

		UWidget* RootWidgetToFocus = UGunsmithBlueprintFunctionLibrary::GetRootFocusWidget(WidgetToFocus);
		if (RootWidgetToFocus)
		{
			UIMode.SetWidgetToFocus(RootWidgetToFocus->GetCachedWidget());
		}

		SetInputMode(UIMode);
	}
	else
	{
		FInputModeGameOnly GameMode;

		SetInputMode(GameMode);
	}
}

void AGunsmithPlayerController::RequestServerRestartPawn()
{
	ServerRestartPlayer();
}

void AGunsmithPlayerController::Server_KillPawn_Implementation()
{
#if !UE_BUILD_SHIPPING
	if (UGSHealthComponent* HealthComponent = UGSGameplayLibrary::GetHealthComponentFromActor(GetPawn()))
	{
		FGSDamageRequest DamageRequest;
		DamageRequest.BaseDamage = 999999.0f;
		DamageRequest.Instigator = this;
		DamageRequest.DamageCauser = GetPawn();
		
		HealthComponent->ApplyDamage(DamageRequest);
	}
#endif
}

void AGunsmithPlayerController::Server_EquipWeapon_Implementation(UGSWeaponDataAsset* WeaponData)
{
	if (UGSShootingComponent* ShootingComponent = UGSGameplayLibrary::GetShootingComponentFromActor(GetPawn()))
	{
		FGSEquipData NewData;
		NewData.WeaponData = WeaponData;
				
		ShootingComponent->EquipWeapon(NewData, ShootingComponent->GetActiveWeaponSlot());
	}
}

void AGunsmithPlayerController::OnPausePressed(const FInputActionValue& Value)
{
	if (AGunsmithHUD* GunsmithHUD = GetHUD<AGunsmithHUD>())
	{
		SetPaused(GunsmithHUD->HasAnyActiveWidgets());
	}
}

void AGunsmithPlayerController::UpdateSpectatorState()
{
	APawn* CurrentPawn = GetPawn();
	ChangeState(CurrentPawn == nullptr ? NAME_Spectating : NAME_Playing);
}

void AGunsmithPlayerController::OnCancelPressed(const FInputActionValue& Value)
{
	if (AGunsmithHUD* GunsmithHUD = GetHUD<AGunsmithHUD>())
	{
		GunsmithHUD->DeactivateTopWidget();
	}
}

void AGunsmithPlayerController::OnHardwareDeviceChanged(const FPlatformUserId UserId, const FInputDeviceId DeviceId)
{
	if (UserId == GetPlatformUserId())
	{
		if (UInputDeviceSubsystem* InputDeviceSubsystem = GEngine->GetEngineSubsystem<UInputDeviceSubsystem>())
		{
			const FHardwareDeviceIdentifier& MostRecentDevice = InputDeviceSubsystem->GetMostRecentlyUsedHardwareDevice(GetPlatformUserId());
			const bool bIsUsingGamepad = MostRecentDevice.PrimaryDeviceType == EHardwareDevicePrimaryType::Gamepad;

			if (bIsUsingGamepad != bWasLastUsingGamepad)
			{
				bWasLastUsingGamepad = bIsUsingGamepad;
				OnDeviceChanged.Broadcast(bWasLastUsingGamepad);
			}
		}
	}
}

void AGunsmithPlayerController::OnControlledPawnDeath(UGSHealthComponent* HealthComponent,
	const FGSDamageRecord& DamageRecord, bool bIsPredicted)
{
	if (!bIsPredicted)
	{
		ChangeState(NAME_Spectating);
	}
}
