// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/GunsmithPlayerController.h"
#include "GunsmithMultiplayerPC.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FGunsmithLobbyOwnershipChangedDelegate, bool bIsOwner);

/**
 *	A custom player controller used in the multiplayer game mode for lobby ownership
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithMultiplayerPC : public AGunsmithPlayerController
{
	GENERATED_BODY()
	
public:
	FGunsmithLobbyOwnershipChangedDelegate OnLobbyOwnershipChanged;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// Sets the match state to InProgress
	UFUNCTION(BlueprintCallable)
	void StartGame();
	
	void SetLobbyOwner(bool bLobbyOwner);
	bool GetIsLobbyOwner() const { return bIsLobbyOwner; }

protected:
	UPROPERTY(ReplicatedUsing=OnRep_bIsLobbyOwner)
	bool bIsLobbyOwner = false;

private:
	UFUNCTION()
	void OnRep_bIsLobbyOwner();

	UFUNCTION(Server, Reliable)
	void Server_StartGame();
};
