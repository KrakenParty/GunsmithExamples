// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GunsmithMoverCharacter.h"
#include "GunsmithGASMoverCharacter.generated.h"

/**
 *	A GAS variant of the Gunsmith Mover Character used to test the Gunsmith GAS integration
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithGASMoverCharacter : public AGunsmithMoverCharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AGunsmithGASMoverCharacter(const FObjectInitializer& ObjectInitializer);
	
	// AGunsmithMoverCharacter Begin
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	// AGunsmithMoverCharacter End

	// IAbilitySystemInterface Begin
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	// IAbilitySystemInterface End

private:
	void SetupAbilitySystemComponent();
};
