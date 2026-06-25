// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/GSWeaponDataAsset.h"
#include "GunsmithExampleWeaponDataAsset.generated.h"

class UGSCharacterAnimationData;

/**
 *	An example of how to extend the Gunsmith Weapon Data Asset
 */
UCLASS()
class GUNSMITHEXAMPLES_API UGunsmithExampleWeaponDataAsset : public UGSWeaponDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon Data Asset|Cosmetic|Animation|Character", DisplayName="Character Anim Data")
	TSubclassOf<UGSCharacterAnimationData> AnimationData = nullptr;
};
