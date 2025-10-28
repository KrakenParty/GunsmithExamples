// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/Modes/GAS/GunsmithAttributeSet.h"

#include "Net/UnrealNetwork.h"

void UGunsmithAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UGunsmithAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGunsmithAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UGunsmithAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

void UGunsmithAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGunsmithAttributeSet, Health, OldHealth);
}

void UGunsmithAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGunsmithAttributeSet, MaxHealth, OldMaxHealth);
}
