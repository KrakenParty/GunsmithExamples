// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/Modes/Range/GunsmithRangePlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Game/Modes/Range/GunsmithGameState_Range.h"
#include "Game/Modes/Range/GunsmithRangeGameMode.h"

void AGunsmithRangePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (RangeMappingContext)
	{
		if (UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			EnhancedInputSubsystem->AddMappingContext(RangeMappingContext, 0);
		}
	}
	
	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (TogglePractiseInputAction)
		{
			Input->BindAction(TogglePractiseInputAction, ETriggerEvent::Started, this, &AGunsmithRangePlayerController::OnTogglePractisePressed);
		}
	}
}

void AGunsmithRangePlayerController::OnTogglePractisePressed(const FInputActionValue& Value)
{
	AGunsmithRangeGameMode* GameMode = GetWorld()->GetAuthGameMode<AGunsmithRangeGameMode>();
	AGunsmithGameState_Range* GameState = GetWorld()->GetGameState<AGunsmithGameState_Range>();
	if (GameMode && GameState)
	{
		if (GameState->IsPractiseActive())
		{
			GameMode->EndPractise();
		}
		else
		{
			GameMode->StartPractise();
		}
	}
}
