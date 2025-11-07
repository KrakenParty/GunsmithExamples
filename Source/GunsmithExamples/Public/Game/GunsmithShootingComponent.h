// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/GSShootingComponent.h"
#include "GunsmithShootingComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGunsmithShowUIDelegate, bool, bShow);

/**
 *	An example of how to extend the default Gunsmith Shooting Component
 */
UCLASS()
class GUNSMITHEXAMPLES_API UGunsmithShootingComponent : public UGSShootingComponent
{
	GENERATED_BODY()
	
public:
	// UGSShootingComponent Begin
	virtual void BeginPlay() override;
	// UGSShootingComponent End
	
	static FGunsmithShowUIDelegate GlobalShowUIDelegate;
	
	UPROPERTY(BlueprintAssignable, Category = "Gunsmith")
	FGunsmithShowUIDelegate ShowUIDelegate;

protected:
	UFUNCTION()
	void OnGlobalShowUI(bool bShow);
};
