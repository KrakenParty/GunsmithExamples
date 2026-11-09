// Copyright Kraken Party Limited. All Rights Reserved.

#include "GunsmithBlueprintFunctionLibrary.h"

#include "MoverComponent.h"
#include "NetworkPredictionWorldManager.h"
#include "PBDRigidsSolver.h"
#include "Blueprint/UserWidget.h"
#include "Character/GSMoverComponent.h"
#include "Engine/DemoNetDriver.h"
#include "GameFramework/PlayerController.h"
#include "Physics/NetworkPhysicsComponent.h"
#include "World/GSWorldStateSubsystem.h"

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
	if (!IsValid(WorldContextObject))
	{
		return 0.0f;
	}
	
	UWorld* World = WorldContextObject->GetWorld();
	
	if (!World)
	{
		return 0.0f;
	}
	
	const UGSWorldStateSubsystem* WorldStateSubsystem = World->GetSubsystem<UGSWorldStateSubsystem>();
	if (WorldStateSubsystem && WorldStateSubsystem->GetUsesAsyncPhysics())
	{
		const FPhysScene* PhysScene = World->GetPhysicsScene();
		const Chaos::FPhysicsSolver* Solver = PhysScene ? PhysScene->GetSolver() : nullptr;
		if (Solver && Solver->IsUsingAsyncResults())
		{
			return static_cast<float>(UE::NetworkPhysicsUtils::GetUpcomingServerFrame_External(World) * Solver->GetAsyncDeltaTime());
		}
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
