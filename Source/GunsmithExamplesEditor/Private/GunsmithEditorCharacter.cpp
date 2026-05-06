// Copyright Kraken Party Limited. All Rights Reserved.

#include "GunsmithEditorCharacter.h"

#include "GunsmithEditorShootingComponent.h"

AGunsmithEditorCharacter::AGunsmithEditorCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass(GSCharacterNames::ShootingComponentName, UGunsmithEditorShootingComponent::StaticClass()))
{
	
}
