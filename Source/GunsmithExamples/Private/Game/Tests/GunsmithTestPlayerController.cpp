// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/Tests/GunsmithTestPlayerController.h"

#include "Engine/World.h"
#include "Game/Tests/GunsmithGameState_Accuracy.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DefaultValueHelper.h"

FAutoConsoleCommandWithWorldAndArgs FCmdGunsmithStartAccuracyTest
(
	TEXT("Gunsmith.Tests.Accuracy"),
	TEXT("Once set, characters will shoot at the target character index and record hit rate. Use -1 to stop again. Optional second parameter to specify the bone name to shoot at."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& InParams, const UWorld* InWorld)
	{
		if(InParams.Num() == 0 || !InWorld)
		{
			return;
		}

		if (AGunsmithTestPlayerController* PlayerController = Cast<AGunsmithTestPlayerController>(UGameplayStatics::GetPlayerController(InWorld, 0)))
		{
			int32 TargetCharacterIndex = INDEX_NONE;
			if (FDefaultValueHelper::ParseInt(InParams[0], TargetCharacterIndex))
			{
				const FName BoneName = InParams.IsValidIndex(1) ? FName(InParams[1]) : NAME_None;
				PlayerController->Server_SetAutoShootIndex(TargetCharacterIndex, BoneName);
			}	
		}
	}),
	ECVF_Cheat
);

void AGunsmithTestPlayerController::Server_SetAutoShootIndex_Implementation(int32 Index, FName BoneName)
{
	if (AGunsmithGameState_Accuracy* GameState_Accuracy = GetWorld()->GetGameState<AGunsmithGameState_Accuracy>())
	{
		GameState_Accuracy->SetAutoShootData(Index, BoneName);
	}
	else
	{
		UE_LOG(LogGunsmithTests, Log, TEXT("Unable to start test as the Game State is not derived from GunsmithGameState_Accuracy"));	
	}
}
