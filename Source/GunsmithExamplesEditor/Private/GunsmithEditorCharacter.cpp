// Copyright Kraken Party Limited. All Rights Reserved.

#include "GunsmithEditorCharacter.h"

#include "GunsmithEditorShootingComponent.h"
#include "Weapon/GSWeaponTypeTags.h"

namespace GunsmithEditorCharacterNames
{
	static const FName LeftHandSocket = TEXT("weapon_l");
};

AGunsmithEditorCharacter::AGunsmithEditorCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass(GSCharacterNames::ShootingComponentName, UGunsmithEditorShootingComponent::StaticClass()))
{
	
}

FName AGunsmithEditorCharacter::GetWeaponAttachmentSocketName_Implementation(
	const UGSWeaponDataAsset* WeaponDataAsset) const
{
	if (WeaponDataAsset->WeaponType == TAG_Weapon_Type_Grenade)
	{
		return GunsmithEditorCharacterNames::LeftHandSocket;
	}
	
	return Super::GetWeaponAttachmentSocketName_Implementation(WeaponDataAsset);
}
