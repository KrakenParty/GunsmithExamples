// Copyright Kraken Party Limited. All Rights Reserved.

#include "GunsmithEditorShootingComponent.h"

#include "Game/Effects/NumberPops/LyraNumberPopComponent.h"
#include "Game/Effects/NumberPops/LyraNumberPopComponent_NiagaraText.h"
#include "Health/GSHealthComponent.h"

void UGunsmithEditorShootingComponent::NotifyDamageDealt_Implementation(const FGSDamageRecord& DamageRecord, bool bIsPredicted)
{
	Super::NotifyDamageDealt_Implementation(DamageRecord, bIsPredicted);
	
	// Lazy create a number pop component which is owned by this component/actor as the controller does not exist in the editor
	if (!NumberPopComponent)
	{
		NumberPopComponent = Cast<ULyraNumberPopComponent_NiagaraText>(GetOwner()->AddComponentByClass(ULyraNumberPopComponent_NiagaraText::StaticClass(), false, FTransform::Identity, false));
		NumberPopComponent->SetStyle(NumberPopStyle);
	}
	
	FLyraNumberPopRequest Request;
	Request.WorldLocation = DamageRecord.HitLocation;
	Request.NumberToDisplay = FMath::RoundToInt(DamageRecord.Damage);
	Request.bIsCriticalDamage = DamageRecord.DamageMultiplier > 1.5f;
		
	NumberPopComponent->AddNumberPop(Request);
}
