// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/GunsmithMultiplayerGameMode.h"

#include "EngineUtils.h"
#include "GSGameplayLibrary.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Game/GunsmithMultiplayerGameState.h"
#include "Game/GunsmithMultiplayerPC.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "Health/GSHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/GSShootingComponent.h"

void AGunsmithMultiplayerGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!LobbyOwner.IsValid())
	{
		if (AGunsmithMultiplayerPC* MultiplayerPC = Cast<AGunsmithMultiplayerPC>(NewPlayer))
		{
			LobbyOwner = MultiplayerPC;
			MultiplayerPC->SetLobbyOwner(true);
		}
	}
}

void AGunsmithMultiplayerGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (Exiting == LobbyOwner)
	{
		LobbyOwner = nullptr;
	}

	// Assign a new owner at random
	if (!LobbyOwner.IsValid())
	{
		const int32 NumControllers = UGameplayStatics::GetNumPlayerControllers(this) - 1;

		if (NumControllers > 0)
		{
			int32 ControllerIndex = FMath::RandRange(0, NumControllers - 1);
			LobbyOwner = Cast<AGunsmithMultiplayerPC>(UGameplayStatics::GetPlayerController(this, ControllerIndex));

			if (LobbyOwner == Exiting)
			{
				LobbyOwner = Cast<AGunsmithMultiplayerPC>(UGameplayStatics::GetPlayerController(this, FMath::WrapExclusive(ControllerIndex + 1, 0, NumControllers + 1)));
			}

			if (ensure(LobbyOwner.IsValid()))
			{
				LobbyOwner->SetLobbyOwner(true);
			}
		}
	}
}

AActor* AGunsmithMultiplayerGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	if (AvailablePlayerStarts.Num() == 0)
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	const int32 RandomStartIndex = FMath::RandRange(0, AvailablePlayerStarts.Num() - 1);
	APlayerStart* PlayerStart = AvailablePlayerStarts[RandomStartIndex].Get();
	AvailablePlayerStarts.RemoveAt(RandomStartIndex, 1);

	return PlayerStart;
}

bool AGunsmithMultiplayerGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	// Prevent players from starting in the same spot every round
	return false;
}

void AGunsmithMultiplayerGameMode::HandleMatchHasStarted()
{
	bShouldAutoRespawn = false;
	RestartRound();
	
	Super::HandleMatchHasStarted();
}

bool AGunsmithMultiplayerGameMode::ReadyToStartMatch_Implementation()
{
	return false;
}

void AGunsmithMultiplayerGameMode::OnPawnDeath(UGSHealthComponent* HealthComponent, const FGSDamageRecord& DamageRecord, bool bIsPredicted)
{
	int32 AlivePlayers = 0;
	
	AGameStateBase* GameStateBase = GetGameState<AGameStateBase>();

	// Check how many players are still alive
	if (GameStateBase)
	{
		for (APlayerState* PlayerState : GameStateBase->PlayerArray)
		{
			APawn* PlayerPawn = PlayerState->GetPawn();
			UGSHealthComponent* PawnHealthComponent = UGSGameplayLibrary::GetHealthComponentFromActor(PlayerPawn);

			if (PawnHealthComponent && !PawnHealthComponent->IsDead())
			{
				AlivePlayers++;
			}
		}
	}

	if (AlivePlayers < 2)
	{
		if (EndOfRoundTime > 0.0f)
		{
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateUObject(this, &AGunsmithMultiplayerGameMode::RestartRound), EndOfRoundTime, false);
		}
		else
		{
			RestartRound();
		}
	}
}

void AGunsmithMultiplayerGameMode::RestartRound()
{
	if (!ensureAlways(WeaponPool.Num() != 0))
	{
		return;
	}

	// Update the player start list
	AvailablePlayerStarts.Reset();
	
	UWorld* World = GetWorld();
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		APlayerStart* PlayerStart = *It;
		AvailablePlayerStarts.Emplace(PlayerStart);
	}
	
	TArray<TSubclassOf<UGSWeaponAttachment>> TempPool = AttachmentPool;

	// Create the random equip data
	FGSEquipData EquipData;
	EquipData.WeaponData = WeaponPool[FMath::RandRange(0, WeaponPool.Num() - 1)];

	for (int32 i = 0; i < NumAttachments; i++)
	{
		if (TempPool.Num() == 0)
		{
			break;
		}
		
		const int32 RandomAttachmentIndex = FMath::RandRange(0, TempPool.Num() - 1);
		EquipData.Attachments.Emplace(TempPool[RandomAttachmentIndex], 1, -1, -1);

		TempPool.RemoveAt(RandomAttachmentIndex, 1);
	}

	// Update the game state with the new round equipment
	if (AGunsmithMultiplayerGameState* GSGameState = GetGameState<AGunsmithMultiplayerGameState>())
	{
		GSGameState->SetRoundEquipment(EquipData);
	}
	
	const int32 NumControllers = UGameplayStatics::GetNumPlayerControllers(this);

	for (int32 i = 0; i < NumControllers; i++)
	{
		APlayerController* Controller = UGameplayStatics::GetPlayerController(this, i);

		if (Controller)
		{
			if (APawn* ExistingPawn = Controller->GetPawn())
			{
				ExistingPawn->Destroy();
			}
			
			RestartPlayer(Controller);

			// Equip the round weapon
			if (UGSShootingComponent* ShootingComponent = UGSGameplayLibrary::GetShootingComponentFromActor(Controller->GetPawn()))
			{
				ShootingComponent->UnequipAllWeapons();
				ShootingComponent->EquipWeapon(EquipData);
			}
		}
	}
}