// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/GSShootingComponent.h"
#include "GunsmithEditorShootingComponent.generated.h"

class ULyraNumberPopComponent_NiagaraText;
class ULyraDamagePopStyleNiagara;

/**
 *	A custom editor shooting component used to route hits to a local number pop component so they can be shown in editor
 */
UCLASS()
class UGunsmithEditorShootingComponent : public UGSShootingComponent
{
	GENERATED_BODY()
	
protected:
	// UGSShootingComponent Begin
	virtual void NotifyDamageDealt_Implementation(const FGSDamageRecord& DamageRecord, bool bIsPredicted) override;
	// UGSShootingComponent End
	
	UPROPERTY(EditDefaultsOnly, Category = "Number Pop")
	TObjectPtr<ULyraDamagePopStyleNiagara> NumberPopStyle;
	
	UPROPERTY()
	TObjectPtr<ULyraNumberPopComponent_NiagaraText> NumberPopComponent = nullptr;
};
