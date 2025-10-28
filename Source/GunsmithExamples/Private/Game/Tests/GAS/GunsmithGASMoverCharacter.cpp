// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/Tests/GAS/GunsmithGASMoverCharacter.h"

#include "Game/Modes/GAS/GunsmithGASHealthComponent.h"
#include "Game/Tests/GAS/GunsmithGASPlayerState.h"
#include "Weapon/GSShootingComponent.h"

AGunsmithGASMoverCharacter::AGunsmithGASMoverCharacter(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer.SetDefaultSubobjectClass<UGunsmithGASHealthComponent>(GET_MEMBER_NAME_CHECKED(AGunsmithGASMoverCharacter, HealthComponent)))
{
	
}

void AGunsmithGASMoverCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (NewController)
	{
		SetupAbilitySystemComponent();
	}
}

void AGunsmithGASMoverCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (GetPlayerState())
	{
		SetupAbilitySystemComponent();
	}
}

UAbilitySystemComponent* AGunsmithGASMoverCharacter::GetAbilitySystemComponent() const
{
	if (AGunsmithGASPlayerState* GasPlayerState = GetPlayerState<AGunsmithGASPlayerState>())
	{
		return GasPlayerState->GetAbilitySystemComponent();
	}

	return nullptr;
}

void AGunsmithGASMoverCharacter::SetupAbilitySystemComponent()
{
	AGunsmithGASPlayerState* GasPlayerState = GetPlayerState<AGunsmithGASPlayerState>();
	if (ensure(GasPlayerState))
	{
		GasPlayerState->InitASC(this);

		ShootingComponent->SetupAbilityComponent(GasPlayerState->GetAbilitySystemComponent());
	}
}
