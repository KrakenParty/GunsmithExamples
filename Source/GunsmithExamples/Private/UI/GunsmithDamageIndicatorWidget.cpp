// Copyright Kraken Party Limited. All Rights Reserved.


#include "UI/GunsmithDamageIndicatorWidget.h"

#include "GSGameplayLibrary.h"
#include "GunsmithMoverCharacter.h"
#include "Health/GSHealthComponent.h"


void UGunsmithDamageIndicatorWidget::SetupForCharacter(const AGunsmithMoverCharacter* Character)
{
	UGSHealthComponent* HealthComponent = UGSGameplayLibrary::GetHealthComponentFromActor(Character);

	if (!HealthComponent)
	{
		SetHealthViewModel(nullptr);
		return;
	}

	SetHealthViewModel(HealthComponent->GetHealthViewModel());
}
