// Copyright Kraken Party Limited. All Rights Reserved.

#include "GunsmithBlueprintFunctionLibrary.h"

#include "MoverComponent.h"
#include "NetworkPredictionWorldManager.h"
#include "Blueprint/UserWidget.h"
#include "Character/GSMoverComponent.h"
#include "Engine/DemoNetDriver.h"
#include "GameFramework/PlayerController.h"

ULocalPlayer* UGunsmithBlueprintFunctionLibrary::GetLocalPlayerFromPlayerController(APlayerController* PlayerController)
{
	if (IsValid(PlayerController))
	{
		return PlayerController->GetLocalPlayer();
	}

	return nullptr;
}

float UGunsmithBlueprintFunctionLibrary::GetTimeSinceSimulationStarted(UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return 0.0f;
	}
	
	if (UNetworkPredictionWorldManager* WorldManager = WorldContextObject->GetWorld()->GetSubsystem<UNetworkPredictionWorldManager>())
	{
		return static_cast<float>(WorldManager->GetFixedTickState().GetTotalSimTimeMS()) * 0.0001f;
	}

	return 0.0f;
}

float UGunsmithBlueprintFunctionLibrary::GetRemainderBaseOnTimeSinceSimulationStarted(UObject* WorldContextObject, float Time)
{
	float TimeSinceSimulationStarted = GetTimeSinceSimulationStarted(WorldContextObject);

	if (TimeSinceSimulationStarted > 0.0f)
	{
		return FMath::Fmod(TimeSinceSimulationStarted, Time);
	}

	return 0.0f;
}

UWidget* UGunsmithBlueprintFunctionLibrary::GetRootFocusWidget(UWidget* Widget)
{
	while (true)
	{
		UUserWidget* UserWidgetToFocus = Cast<UUserWidget>(Widget);

		if (!UserWidgetToFocus)
		{
			break;
		}

		UWidget* NewFocusWidget = UserWidgetToFocus->GetDesiredFocusWidget();

		if (!NewFocusWidget)
		{
			break;
		}

		Widget = NewFocusWidget;
	}

	return Widget;
}

bool UGunsmithBlueprintFunctionLibrary::IsReplayPlaying(UObject* WorldContextObject)
{
	if (WorldContextObject)
	{
		if (UWorld* World = WorldContextObject->GetWorld())
		{
			if (UDemoNetDriver* DemoNetDriver = World->GetDemoNetDriver())
			{
				if (DemoNetDriver->IsPlaying())
				{
					return true;
				}
			}
		}
	}

	return false;
}
