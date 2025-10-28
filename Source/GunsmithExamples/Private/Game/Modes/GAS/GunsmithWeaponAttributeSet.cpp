// Copyright Kraken Party Limited. All Rights Reserved.


#include "Game/Modes/GAS/GunsmithWeaponAttributeSet.h"

#include "Net/UnrealNetwork.h"


void UGunsmithWeaponAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UGunsmithWeaponAttributeSet, Damage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGunsmithWeaponAttributeSet, TimeBetweenShots, COND_None, REPNOTIFY_Always);
}

void UGunsmithWeaponAttributeSet::OnRep_Damage(const FGameplayAttributeData& OldDamage)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGunsmithWeaponAttributeSet, Damage, OldDamage);
}

void UGunsmithWeaponAttributeSet::OnRep_TimeBetweenShots(const FGameplayAttributeData& OldTimeBetweenShots)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGunsmithWeaponAttributeSet, TimeBetweenShots, OldTimeBetweenShots);
}
