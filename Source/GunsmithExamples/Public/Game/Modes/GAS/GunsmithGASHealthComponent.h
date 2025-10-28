// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Health/GSHealthComponent.h"
#include "GunsmithGASHealthComponent.generated.h"

struct FOnAttributeChangeData;

/**
 *	A custom health component that reports attributes to the health view model instead of using NPP
 */
UCLASS()
class GUNSMITHEXAMPLES_API UGunsmithGASHealthComponent : public UGSHealthComponent
{
	GENERATED_BODY()

public:	
	// UGSHealthComponent Begin
	virtual void BeginPlay() override;
	virtual void UpdateHealthViewModel_Implementation(const FGSHealthSyncState& Sync, const FGSHealthAuxState& Aux) override;
	virtual void ApplyDamage_Implementation(FGSDamageRequest& Request) override;
	// UGSHealthComponent End

protected:
	UPROPERTY(EditDefaultsOnly, Category="Gunsmith")
	FGameplayAttribute HealthAttribute;

	UPROPERTY(EditDefaultsOnly, Category="Gunsmith")
	FGameplayAttribute MaxHealthAttribute;

	void OnHealthChanged(const FOnAttributeChangeData& ChangeData);
	void OnMaxHealthChanged(const FOnAttributeChangeData& ChangeData) const;
};
