// Copyright Kraken Party Limited. All Rights Reserved.


#include "UI/GunsmithHUDWidget.h"

#include "GSGameplayLibrary.h"
#include "GunsmithMoverCharacter.h"
#include "Weapon/GSShootingComponent.h"
#include "Health/GSHealthComponent.h"
#include "UI/GunsmithDamageIndicatorWidget.h"
#include "UI/GunsmithHealthVignetteWidget.h"

void UGunsmithHUDWidget::SetupForCharacter(const AGunsmithMoverCharacter* Character)
{
	if (UGSHealthComponent* HealthComponent = UGSGameplayLibrary::GetHealthComponentFromActor(Character))
	{
		UGSHealthViewModel* HealthViewModel = HealthComponent->GetHealthViewModel();
		SetHealthViewModel(HealthViewModel);

		if (HealthVignetteWidget)
		{
			HealthVignetteWidget->SetHealthViewModel(HealthViewModel);
		}
	}

	if (UGSShootingComponent* ShootingComponent = UGSGameplayLibrary::GetShootingComponentFromActor(Character))
	{
		SetWeaponViewModel(ShootingComponent->GetWeaponViewModel());	
	}

	if (DamageIndicatorWidget)
	{
		DamageIndicatorWidget->SetupForCharacter(Character);	
	}
}
