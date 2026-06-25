// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GSAnimatedEditorCharacter.h"
#include "GunsmithEditorCharacter.generated.h"

/**
 *	A custom version of the editor character used to enhance the viewport experience
 */
UCLASS()
class AGunsmithEditorCharacter : public AGSAnimatedEditorCharacter
{
	GENERATED_BODY()

public:
	AGunsmithEditorCharacter(const FObjectInitializer& ObjectInitializer);
	
	// IGSShootingInterface Begin
	virtual FName GetWeaponAttachmentSocketName_Implementation(const UGSWeaponDataAsset* WeaponDataAsset) const override;
	// IGSShootingInterface End
};
