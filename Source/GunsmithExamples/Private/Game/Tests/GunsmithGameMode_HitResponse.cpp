// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/Tests/GunsmithGameMode_HitResponse.h"

#include "EngineUtils.h"
#include "Game/GunsmithPriorityPlayerStart.h"
#include "GameFramework/GameStateBase.h"

AActor* AGunsmithGameMode_HitResponse::ChoosePlayerStart_Implementation(AController* Player)
{
	UClass* PawnClass = GetDefaultPawnClassForController(Player);
	APawn* PawnToFit = PawnClass ? PawnClass->GetDefaultObject<APawn>() : nullptr;
	AGameStateBase* GameStateBase = GetGameState<AGameStateBase>();
	const bool bIsFirstPlayer = GameStateBase && !GameStateBase->PlayerArray.IsEmpty() && Player->PlayerState == GameStateBase->PlayerArray[0];

	if (bIsFirstPlayer)
	{
		UWorld* World = GetWorld();
		for (TActorIterator<AGunsmithPriorityPlayerStart> It(World); It; ++It)
		{
			AGunsmithPriorityPlayerStart* PlayerStart = *It;

			FVector ActorLocation = PlayerStart->GetActorLocation();
			const FRotator ActorRotation = PlayerStart->GetActorRotation();
			if (!World->EncroachingBlockingGeometry(PawnToFit, ActorLocation, ActorRotation))
			{
				return PlayerStart;
			}
		}
	}
	
	return Super::ChoosePlayerStart_Implementation(Player);
}
