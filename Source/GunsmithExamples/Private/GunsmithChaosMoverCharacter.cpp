// Copyright Kraken Party Limited. All Rights Reserved.

#include "GunsmithChaosMoverCharacter.h"

#include "Game/GunsmithChaosMoverComponent.h"

AGunsmithChaosMoverCharacter::AGunsmithChaosMoverCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass(GunsmithMoverCharacterNames::CharacterMotionComponent, UGunsmithChaosMoverComponent::StaticClass()))
{
	SetReplicatingMovement(true);
	SetPhysicsReplicationMode(EPhysicsReplicationMode::Resimulation);
}
