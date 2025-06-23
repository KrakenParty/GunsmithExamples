// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/GunsmithGameState.h"
#include "Weapon/GSEquipData.h"
#include "GunsmithMultiplayerGameState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FGunsmithGameStateChangedDelegate, FName NewState);
DECLARE_MULTICAST_DELEGATE_OneParam(FGunsmithGameStateRoundEquipmentChangedDelegate, const FGSEquipData& NewEquipData);

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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// AGunsmithGameState End

	// Sets the active round equipment data for UI to broadcast
	void SetRoundEquipment(const FGSEquipData& NewEquipData);

protected:
	// The current round equipment
	UPROPERTY(ReplicatedUsing=OnRep_CurrentRoundEquipment)
	FGSEquipData CurrentRoundEquipment;
	
	virtual void OnRep_MatchState() override;

private:
	UFUNCTION()
	void OnRep_CurrentRoundEquipment() const;
};
