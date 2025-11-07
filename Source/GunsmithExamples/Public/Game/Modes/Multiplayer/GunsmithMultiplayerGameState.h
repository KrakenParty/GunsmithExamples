// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/GunsmithGameState.h"
#include "Weapon/GSEquipData.h"
#include "GunsmithMultiplayerGameState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FGunsmithGameStateChangedDelegate, FName NewState);
DECLARE_MULTICAST_DELEGATE_OneParam(FGunsmithGameStateRoundEquipmentChangedDelegate, const FGSEquipData& NewEquipData);

USTRUCT()
struct FGSMultiplayerWeaponPoolLoadouts
{
	GENERATED_BODY()

	// The weapon that can be assigned
	UPROPERTY(EditDefaultsOnly, Category = "Gunsmith")
	TObjectPtr<UGSWeaponDataAsset> WeaponData = nullptr;

	// Which attachments can be applied to the weapon
	UPROPERTY(EditDefaultsOnly, Category = "Gunsmith")
	TArray<TSubclassOf<UGSWeaponAttachment>> Attachments;
};

/**
 *	A Game state to accompany the Gunsmith Multiplayer Game Mode
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithMultiplayerGameState : public AGunsmithGameState
{
	GENERATED_BODY()
	
public:
	FGunsmithGameStateChangedDelegate OnStateChangedDelegate;
	FGunsmithGameStateRoundEquipmentChangedDelegate OnEquipmentChangedDelegate;

	// AGunsmithGameState Begin
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// AGunsmithGameState End

	// Sets the active round equipment data for UI to broadcast
	void SetRoundEquipment(const FGSEquipData& NewEquipData);

	const TArray<FGSMultiplayerWeaponPoolLoadouts>& GetWeaponPool() const { return WeaponPool; }
	const TArray<TSubclassOf<UGSWeaponAttachment>>& GetGlobalAttachmentPool() const { return GlobalAttachmentPool; }

protected:
	// The pool of weapons to randomly select from at the start of a round
	UPROPERTY(EditDefaultsOnly, Category = "Mode")
	TArray<FGSMultiplayerWeaponPoolLoadouts> WeaponPool;

	// The global pool of attachments to randomly that can be applied to any weapon
	UPROPERTY(EditDefaultsOnly, Category = "Mode")
	TArray<TSubclassOf<UGSWeaponAttachment>> GlobalAttachmentPool;
	
	// The current round equipment
	UPROPERTY(ReplicatedUsing=OnRep_CurrentRoundEquipment)
	FGSEquipData CurrentRoundEquipment;

	UPROPERTY()
	TObjectPtr<UDataTable> PreloadDataTable = nullptr;
	
	virtual void OnRep_MatchState() override;

private:
	UFUNCTION()
	void OnRep_CurrentRoundEquipment() const;
};
