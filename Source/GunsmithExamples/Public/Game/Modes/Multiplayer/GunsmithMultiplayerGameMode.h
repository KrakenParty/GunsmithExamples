// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/GunsmithGameMode.h"
#include "GunsmithMultiplayerGameMode.generated.h"

class AGunsmithMultiplayerPC;
class UGSWeaponDataAsset;
class UGSWeaponAttachment;
class APlayerStart;
class FOnlineSessionSettings;

/**
 *	A Gunsmith game mode created to test core Gunsmith features in a multiplayer environment
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithMultiplayerGameMode : public AGunsmithGameMode
{
	GENERATED_BODY()

public:
	static FName SessionName;
	static FName SearchParam;
	
protected:
	// How long to wait before spawning a character after death
	UPROPERTY(EditDefaultsOnly, Category = "Mode", meta=(ClampMin="0.0"))
	float EndOfRoundTime = 5.0f;

	// The pool of weapons to randomly select from at the start of a round
	UPROPERTY(EditDefaultsOnly, Category = "Mode")
	TArray<TObjectPtr<UGSWeaponDataAsset>> WeaponPool;

	// The pool of attachments to randomly select from at the start of a round
	UPROPERTY(EditDefaultsOnly, Category = "Mode")
	TArray<TSubclassOf<UGSWeaponAttachment>> AttachmentPool;

	// How many attachments to randomly select for each weapon
	UPROPERTY(EditDefaultsOnly, Category = "Mode", meta=(ClampMin="0"))
	int32 NumAttachments = 2;

	// AGunsmithGameMode Begin
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;
	virtual void HandleMatchHasStarted() override;
	virtual void StartMatch() override;
	virtual bool ReadyToStartMatch_Implementation() override;
	virtual void OnPawnDeath(UGSHealthComponent* HealthComponent, const FGSDamageRecord& DamageRecord, bool bIsPredicted) override;
	// AGunsmithGameMode End

private:
	TWeakObjectPtr<AGunsmithMultiplayerPC> LobbyOwner;
	TArray<TWeakObjectPtr<APlayerStart>> AvailablePlayerStarts;
	
	// Reset all player states for a new round start
	void RestartRound();

	static FOnlineSessionSettings CreateSessionSettings(bool bAllowInvites);
};
