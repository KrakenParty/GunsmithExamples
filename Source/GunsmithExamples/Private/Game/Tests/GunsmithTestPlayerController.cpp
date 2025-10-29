// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/Tests/GunsmithTestPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GSGameplayLibrary.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Game/Tests/GunsmithGameState_Accuracy.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DefaultValueHelper.h"
#include "Weapon/GSShootingComponent.h"
#include "World/GSWorldStateSubsystem.h"

static FAutoConsoleCommandWithWorldAndArgs FCmdGunsmithStartAccuracyTest
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

void AGunsmithTestPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (TestControllerMappingContext)
	{
		if (UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			EnhancedInputSubsystem->AddMappingContext(TestControllerMappingContext, 0);
		}
	}
	
	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (AddAttachmentInputAction)
		{
			Input->BindAction(AddAttachmentInputAction, ETriggerEvent::Started, this, &AGunsmithTestPlayerController::OnAddAttachmentPressed);
		}

		if (RemoveAttachmentInputAction)
		{
			Input->BindAction(RemoveAttachmentInputAction, ETriggerEvent::Started, this, &AGunsmithTestPlayerController::OnRemoveAttachmentPressed);
		}
	}
}

void AGunsmithTestPlayerController::OnAddAttachmentPressed(const FInputActionValue& Value)
{
	int32 Frame = -1;
	if (UGSWorldStateSubsystem* WorldStateSubsystem = GetWorld()->GetSubsystem<UGSWorldStateSubsystem>())
	{
		Frame = WorldStateSubsystem->GetCurrentSimulationFrameIndex().ServerFrame;
	}

	Server_AddAttachment_Implementation(Frame);

	if (GetWorld()->IsNetMode(NM_Client))
	{
		Server_AddAttachment(Frame);
	}
}

void AGunsmithTestPlayerController::OnRemoveAttachmentPressed(const FInputActionValue& Value)
{
	int32 Frame = -1;
	if (UGSWorldStateSubsystem* WorldStateSubsystem = GetWorld()->GetSubsystem<UGSWorldStateSubsystem>())
	{
		Frame = WorldStateSubsystem->GetCurrentSimulationFrameIndex().ServerFrame;
	}
	
	Server_RemoveAttachment_Implementation(Frame);

	if (GetWorld()->IsNetMode(NM_Client))
	{
		Server_RemoveAttachment(Frame);
	}
}

void AGunsmithTestPlayerController::Server_RemoveAttachment_Implementation(int32 Frame)
{
	if (UGSShootingComponent* ShootingComponent = UGSGameplayLibrary::GetShootingComponentFromActor(GetPawn()))
	{
		for (const FGSEquipAttachmentData& Attachment : ShootingComponent->GetGlobalAttachments())
		{
			if (Attachment.Attachment == TestAttachmentData.Attachment)
			{
				UE_LOG(LogTemp, Log, TEXT("Removed attachment"));
				
				ShootingComponent->RemoveAttachment(Attachment.AttachmentID, Frame);
				break;
			}
		}
	}
}

void AGunsmithTestPlayerController::Server_AddAttachment_Implementation(int32 Frame)
{
	if (!TestAttachmentData.Attachment)
	{
		return;
	}
	
	FGSEquipAttachmentData NewAttachmentData = TestAttachmentData;
	NewAttachmentData.AppliedFrame = Frame;

	if (UGSShootingComponent* ShootingComponent = UGSGameplayLibrary::GetShootingComponentFromActor(GetPawn()))
	{
		UE_LOG(LogTemp, Log, TEXT("Added attachment"));
		
		ShootingComponent->AddAttachment(NewAttachmentData);
	}
}
