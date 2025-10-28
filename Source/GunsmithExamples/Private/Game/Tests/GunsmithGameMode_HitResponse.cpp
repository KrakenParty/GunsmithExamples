// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/Tests/GunsmithGameMode_HitResponse.h"

#include "EngineUtils.h"
#include "GunsmithMoverCharacter.h"
#include "Game/GunsmithPriorityPlayerStart.h"
#include "GameFramework/GameStateBase.h"
#include "Misc/CommandLine.h"

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

APawn* AGunsmithGameMode_HitResponse::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	APawn* NewPawn = Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);

	// Force all players except the first to run continuously
	static bool bCmdLineNoLoadingScreen = FParse::Param(FCommandLine::Get(), TEXT("AutoRun"));
	if (bCmdLineNoLoadingScreen)
	{
		AGameStateBase* GameStateBase = GetGameState<AGameStateBase>();
		const bool bIsFirstPlayer = GameStateBase && !GameStateBase->PlayerArray.IsEmpty() && NewPlayer->PlayerState == GameStateBase->PlayerArray[0];

		if (!bIsFirstPlayer)
		{
			if (AGunsmithMoverCharacter* GunsmithMoverCharacter = Cast<AGunsmithMoverCharacter>(NewPawn))
			{
				GunsmithMoverCharacter->ServerStartDebugMovement(1);
			}
		}
	}

	return NewPawn;
}
