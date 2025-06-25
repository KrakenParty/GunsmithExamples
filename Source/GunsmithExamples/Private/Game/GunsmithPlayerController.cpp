// Copyright Kraken Party Limited. All Rights Reserved.


#include "Game/GunsmithPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GunsmithBlueprintFunctionLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "GameFramework/InputDeviceSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Netcode/GSNetworkLibrary.h"
#include "UI/GunsmithHUD.h"

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

void AGunsmithPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (CommonInputs)
	{
		UGunsmithCommonInputs* CommonInputObject = CommonInputs->GetDefaultObject<UGunsmithCommonInputs>();
		if (CommonInputObject->UIInputMappingContext)
		{
			if (UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
			{
				EnhancedInputSubsystem->AddMappingContext(CommonInputObject->UIInputMappingContext, 0);
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

ASpectatorPawn* AGunsmithPlayerController::SpawnSpectatorPawn()
{
	// Server is spawned with the correct rotation, client never receives this as ControlRotation is not replicated
	if (GetWorld()->IsNetMode(NM_Client))
	{
		SetControlRotation(SpawnRotation);
	}
	
	return Super::SpawnSpectatorPawn();
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

void AGunsmithPlayerController::OnPausePressed(const FInputActionValue& Value)
{
	if (AGunsmithHUD* GunsmithHUD = GetHUD<AGunsmithHUD>())
	{
		SetPaused(GunsmithHUD->HasAnyActiveWidgets());
	}
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
