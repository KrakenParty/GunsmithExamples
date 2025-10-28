// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "GunsmithWeaponAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * An example attribute set to show how to use GAS in Gunsmith
 */
UCLASS()
class GUNSMITHEXAMPLES_API UGunsmithWeaponAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_Damage)
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UGunsmithWeaponAttributeSet, Damage)

	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_TimeBetweenShots)
	FGameplayAttributeData TimeBetweenShots;
	ATTRIBUTE_ACCESSORS(UGunsmithWeaponAttributeSet, TimeBetweenShots)
	
private:
	UFUNCTION()
	virtual void OnRep_Damage(const FGameplayAttributeData& OldDamage);

	UFUNCTION()
	virtual void OnRep_TimeBetweenShots(const FGameplayAttributeData& OldTimeBetweenShots);
};
