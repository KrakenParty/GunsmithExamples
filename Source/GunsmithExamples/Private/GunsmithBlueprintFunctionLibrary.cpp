// Copyright Kraken Party Limited. All Rights Reserved.

#include "GunsmithBlueprintFunctionLibrary.h"

#include "NetworkPredictionWorldManager.h"
#include "PBDRigidsSolver.h"
#include "Animation/AnimInstance.h"
#include "Blueprint/UserWidget.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DemoNetDriver.h"
#include "Game/Effects/LyraContextEffectsInterface.h"
#include "GameFramework/PlayerController.h"
#include "Physics/NetworkPhysicsComponent.h"
#include "World/GSWorldStateSubsystem.h"
#include "Runtime/Launch/Resources/Version.h"

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
	
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 7
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
#endif
	
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

void UGunsmithBlueprintFunctionLibrary::PlayContextEffect(UAnimInstance* AnimInstance, FGameplayTag Effect, FName SocketName,
	bool bAttached, bool bPerformTrace, FVector TraceEndOffset, TEnumAsByte<ECollisionChannel> TraceChannel, bool bIgnoreOwner,
	FVector LocationOffset, FRotator RotationOffset, FVector VFXScale, float AudioVolume, float AudioPitch)
{
	USkeletalMeshComponent* MeshComp = AnimInstance ? AnimInstance->GetSkelMeshComponent() : nullptr;
	AActor* OwningActor = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!OwningActor)
	{
		return;
	}

	bool bHitSuccess = false;
	FHitResult HitResult;

	if (bPerformTrace)
	{
		if (UWorld* World = OwningActor->GetWorld())
		{
			FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(GunsmithContextEffectTrace));
			QueryParams.bReturnPhysicalMaterial = true;
			if (bIgnoreOwner)
			{
				QueryParams.AddIgnoredActor(OwningActor);
			}

			const FVector TraceStart = bAttached ? MeshComp->GetSocketLocation(SocketName) : MeshComp->GetComponentLocation();
			bHitSuccess = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceStart + TraceEndOffset, TraceChannel, QueryParams);
		}
	}

	// The owning actor and any of its components can handle the effect
	TArray<UObject*> Handlers;
	if (OwningActor->Implements<ULyraContextEffectsInterface>())
	{
		Handlers.Add(OwningActor);
	}
	for (UActorComponent* Component : OwningActor->GetComponents())
	{
		if (Component && Component->Implements<ULyraContextEffectsInterface>())
		{
			Handlers.Add(Component);
		}
	}

	const FName AttachName = bAttached ? SocketName : NAME_None;
	for (UObject* Handler : Handlers)
	{
		// Named notify events don't pass the animation; the effect component doesn't use it
		ILyraContextEffectsInterface::Execute_AnimMotionEffect(Handler, AttachName, Effect, MeshComp, LocationOffset, RotationOffset,
			nullptr, bHitSuccess, HitResult, FGameplayTagContainer(), VFXScale, AudioVolume, AudioPitch);
	}
}
