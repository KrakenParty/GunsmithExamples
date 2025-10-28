// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Game/GunsmithPlayerState.h"
#include "GunsmithGASPlayerState.generated.h"

class UAbilitySystemComponent;
class UGunsmithAttributeSet;
class UGunsmithWeaponAttributeSet;

/**
 *	A player state used to test the Gunsmith GAS integration
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithGASPlayerState : public AGunsmithPlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AGunsmithGASPlayerState();

	// IAbilitySystemInterface Begin
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	// IAbilitySystemInterface End

	void InitASC(AActor* AvatarPawn);

protected:
	UPROPERTY(EditAnywhere, Category="Gunsmith")
	float MaxHealth = 100.0f;
	
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

	UPROPERTY()
	TObjectPtr<UGunsmithAttributeSet> AttributeSet = nullptr;
	UPROPERTY()
	TObjectPtr<UGunsmithWeaponAttributeSet> WeaponAttributeSet = nullptr;
};