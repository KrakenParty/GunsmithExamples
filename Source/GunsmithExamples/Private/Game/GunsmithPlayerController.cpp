// Copyright Kraken Party Limited. All Rights Reserved.


#include "Game/GunsmithPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GunsmithBlueprintFunctionLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "GameFramework/InputDeviceSubsystem.h"
#include "UI/GunsmithHUD.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_InputMode_UI, "EnhancedInput.Modes.UI")

void AGunsmithPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UInputDeviceSubsystem* InputDeviceSubsystem = GEngine->GetEngineSubsystem<UInputDeviceSubsystem>())
	{
		InputDeviceSubsystem->OnInputHardwareDeviceChanged.AddDynamic(this, &AGunsmithPlayerController::OnHardwareDeviceChanged);
	}
}

void AGunsmithPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (PauseInputAction)
		{
			Input->BindAction(PauseInputAction, ETriggerEvent::Started, this, &AGunsmithPlayerController::OnPausePressed);
		}
	}
}

bool AGunsmithPlayerController::ShouldShowMouseCursor() const
{
	return bIsPaused && !bWasLastUsingGamepad;
}

void AGunsmithPlayerController::SetPaused(bool bPaused)
{
	bIsPaused = bPaused;

	UWidget* WidgetToFocus = nullptr;
	
	if (AGunsmithHUD* GunsmithHUD = GetHUD<AGunsmithHUD>())
	{
		WidgetToFocus = GunsmithHUD->SetPaused(bIsPaused);
	}

	SetUIInputMode(bIsPaused, WidgetToFocus);
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
	SetPaused(!bIsPaused);
}

void AGunsmithPlayerController::OnHardwareDeviceChanged(const FPlatformUserId UserId, const FInputDeviceId DeviceId)
{
	if (UserId == GetPlatformUserId())
	{
		if (UInputDeviceSubsystem* InputDeviceSubsystem = GEngine->GetEngineSubsystem<UInputDeviceSubsystem>())
		{
			const FHardwareDeviceIdentifier& MostRecentDevice = InputDeviceSubsystem->GetMostRecentlyUsedHardwareDevice(GetPlatformUserId());
			bWasLastUsingGamepad = MostRecentDevice.PrimaryDeviceType == EHardwareDevicePrimaryType::Gamepad;
		}
	}
}
