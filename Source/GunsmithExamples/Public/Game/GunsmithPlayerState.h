// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GunsmithPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGunsmithPlayerNameChangedDelegate, const FString&, NewName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGunsmithPlayerIdChangedDelegate, APlayerState*, Player);

/**
 *	An example player state class
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	// APlayerState Begin
	virtual void OnRep_PlayerName() override;
	virtual void OnRep_PlayerId() override;
	// APlayerState End

	UPROPERTY(BlueprintAssignable, Category="Gunsmith")
	FGunsmithPlayerNameChangedDelegate OnNameChanged;

	UPROPERTY(BlueprintAssignable, Category="Gunsmith")
	FGunsmithPlayerIdChangedDelegate OnIdChanged;
};
