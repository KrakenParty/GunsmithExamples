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
	// Unbind from old events
	if (CachedCharacter.IsValid())
	{
		if (UGSShootingComponent* ShootingComponent = UGSGameplayLibrary::GetShootingComponentFromActor(CachedCharacter.Get()))
		{
			ShootingComponent->OnViewModelsSetup.RemoveAll(this);
		}
	
		if (UGSShootingComponent* GrenadeComponent = CachedCharacter->GetGrenadeComponent())
		{
			GrenadeComponent->OnViewModelsSetup.RemoveAll(this);
		}
	}
	
	// Bind to new events
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
		
		ShootingComponent->OnViewModelsSetup.AddUniqueDynamic(this, &UGunsmithHUDWidget::OnWeaponVMUpdated);
	}
	
	if (UGSShootingComponent* GrenadeComponent = Character->GetGrenadeComponent())
	{
		SetGrenadeViewModel(GrenadeComponent->GetWeaponViewModel());
		
		GrenadeComponent->OnViewModelsSetup.AddUniqueDynamic(this, &UGunsmithHUDWidget::OnGrenadeVMUpdated);
	}

	if (DamageIndicatorWidget)
	{
		DamageIndicatorWidget->SetupForCharacter(Character);	
	}
	
	CachedCharacter = Character;
}

void UGunsmithHUDWidget::OnWeaponVMUpdated(UGSWeaponViewModel* WeaponVM, UGSCrosshairViewModel* CrosshairVM)
{
	SetWeaponViewModel(WeaponVM);
}

void UGunsmithHUDWidget::OnGrenadeVMUpdated(UGSWeaponViewModel* WeaponVM, UGSCrosshairViewModel* CrosshairVM)
{
	SetGrenadeViewModel(WeaponVM);
}
