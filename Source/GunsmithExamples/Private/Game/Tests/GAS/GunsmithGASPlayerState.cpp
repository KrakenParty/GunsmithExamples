// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/Tests/GAS/GunsmithGASPlayerState.h"

#include "AbilitySystemComponent.h"
#include "Game/Modes/GAS/GunsmithAttributeSet.h"
#include "Game/Modes/GAS/GunsmithWeaponAttributeSet.h"

AGunsmithGASPlayerState::AGunsmithGASPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->ReplicationMode = EGameplayEffectReplicationMode::Mixed;

	AttributeSet = CreateDefaultSubobject<UGunsmithAttributeSet>("GunsmithAttributes");
	WeaponAttributeSet = CreateDefaultSubobject<UGunsmithWeaponAttributeSet>("GunsmithWeaponAttributes");

	SetNetUpdateFrequency(100.0f);
}

void AGunsmithGASPlayerState::InitASC(AActor* AvatarPawn)
{
	AbilitySystemComponent->InitAbilityActorInfo(this, AvatarPawn);

	// Set attribute defaults
	if (AttributeSet)
	{
		AttributeSet->SetMaxHealth(MaxHealth);
		AttributeSet->SetHealth(MaxHealth);
	}
}
