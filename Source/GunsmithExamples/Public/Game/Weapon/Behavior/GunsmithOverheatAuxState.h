// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MoverTypes.h"
#include "UObject/NoExportTypes.h"
#include "GunsmithOverheatAuxState.generated.h"

/**
 *  An example of a custom aux state which can be added to the NPP simulation
 */
USTRUCT(BlueprintType)
struct GUNSMITHEXAMPLES_API FGunsmithOverheatAuxState : public FMoverDataStructBase
{
	GENERATED_BODY()
	
	// How long to wait until the cool down kicks in
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Overheat")
	float TimeUntilCoolDown = 0.0f;

	// FMoverDataStructBase Begin
	virtual FMoverDataStructBase* Clone() const override;
	virtual UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;
	virtual bool ShouldReconcile(const FMoverDataStructBase& AuthorityState) const override;
	virtual void ToString(FAnsiStringBuilderBase& Out) const override;
	virtual void Interpolate(const FMoverDataStructBase& From, const FMoverDataStructBase& To, float Pct) override;
	// FMoverDataStructBase End
};

template<>
struct TStructOpsTypeTraits<FGunsmithOverheatAuxState> : public TStructOpsTypeTraitsBase2<FGunsmithOverheatAuxState>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};