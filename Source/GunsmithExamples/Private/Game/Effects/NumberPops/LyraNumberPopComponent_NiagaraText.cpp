// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/Effects/NumberPops/LyraNumberPopComponent_NiagaraText.h"

#include "GSLog.h"
#include "Game/Effects/NumberPops/LyraNumberPopComponent.h"
#include "Game/Effects/NumberPops/LyraDamagePopStyleNiagara.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraNumberPopComponent_NiagaraText)

ULyraNumberPopComponent_NiagaraText::ULyraNumberPopComponent_NiagaraText(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ULyraNumberPopComponent_NiagaraText::AddNumberPop(const FLyraNumberPopRequest& NewRequest)
{
	if (!FApp::CanEverRender() || !Style)
	{
		return;
	}
	
	int32 LocalDamage = NewRequest.NumberToDisplay;

	//Change Damage to negative to differentiate Critical vs Normal hit
	if (NewRequest.bIsCriticalDamage)
	{
		LocalDamage *= -1;
	}

	//Add a NiagaraComponent if we don't already have one
	if (!NiagaraComp)
	{		
		FFXSystemSpawnParameters SpawnParams;
		SpawnParams.WorldContextObject = this;
		SpawnParams.SystemTemplate = Style->TextNiagara;
		SpawnParams.Location = NewRequest.WorldLocation;
		SpawnParams.bAutoDestroy = false;
		SpawnParams.bAutoActivate = false;
		SpawnParams.PoolingMethod = EPSCPoolMethod::None;
		SpawnParams.bPreCullCheck = true;
		SpawnParams.bIsPlayerEffect = true;
		NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocationWithParams(SpawnParams);
	}

	NiagaraComp->Activate(false);
	NiagaraComp->SetWorldLocation(NewRequest.WorldLocation);

	UE_LOG(LogGunsmith, Verbose, TEXT("DamageHit location : %s"), *(NewRequest.WorldLocation.ToString()));
	
	//Add Damage information to the current Niagara list - Damage informations are packed inside a FVector4 where XYZ = Position, W = Damage
	TArray<FVector4> DamageList = UNiagaraDataInterfaceArrayFunctionLibrary::GetNiagaraArrayVector4(NiagaraComp, Style->NiagaraArrayName);
	DamageList.Add(FVector4(NewRequest.WorldLocation.X, NewRequest.WorldLocation.Y, NewRequest.WorldLocation.Z, LocalDamage));
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector4(NiagaraComp, Style->NiagaraArrayName, DamageList);
}

