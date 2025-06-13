// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GunsmithAnimNotify_Branching.generated.h"

/**
 *	A branching point notify state required to make anim notifies more reliable at the cost of performance
 */
UCLASS()
class GUNSMITHEXAMPLES_API UGunsmithAnimNotify_Branching : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UGunsmithAnimNotify_Branching();
};
