// Copyright Kraken Party Limited. All Rights Reserved.


#include "UI/GunsmithHUD.h"

#include "GSGameplayLibrary.h"
#include "Weapon/GSShootingComponent.h"
#include "Blueprint/UserWidget.h"
#include "UI/GunsmithHUDWidget.h"
#include "GunsmithMoverCharacter.h"
#include "Health/GSHealthComponent.h"
#include "UI/GunsmithPauseMenuWidget.h"


void AGunsmithHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HUDWidgetType)
	{
		HUDWidget = CreateWidget<UGunsmithHUDWidget>(PlayerOwner, HUDWidgetType, TEXT("PlayerHUD"));

		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}

	if (PauseMenuWidgetType)
	{
		PauseMenuWidget = CreateWidget<UGunsmithPauseMenuWidget>(PlayerOwner, PauseMenuWidgetType, TEXT("PauseMenu"));

		if (PauseMenuWidget)
		{
			PauseMenuWidget->AddToViewport(UGSDeveloperSettings::Get()->CrosshairZOrder + 1);
			PauseMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (PlayerOwner)
	{
		PlayerOwner->OnPossessedPawnChanged.AddUniqueDynamic(this, &AGunsmithHUD::OnPossessedPawnChanged);
		OnPossessedPawnChanged(nullptr, PlayerOwner->GetPawn());
	}
}

UWidget* AGunsmithHUD::SetPaused(bool bPaused) const
{
	if (PauseMenuWidget)
	{
		PauseMenuWidget->SetVisibility(bPaused ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

		if (bPaused)
		{
			return PauseMenuWidget->GetDesiredFocusWidget();
		}
	}

	return nullptr;
}

void AGunsmithHUD::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	AGunsmithMoverCharacter* MoverCharacter = Cast<AGunsmithMoverCharacter>(NewPawn);
	if (IsValid(MoverCharacter) && HUDWidget)
	{
		UGSHealthComponent* HealthComponent = UGSGameplayLibrary::GetHealthComponentFromActor(MoverCharacter);
		HUDWidget->SetHealthViewModel(HealthComponent ? HealthComponent->GetHealthViewModel() : nullptr);
		HUDWidget->SetWeaponViewModel(UGSGameplayLibrary::GetShootingComponentFromActor(MoverCharacter)->GetWeaponViewModel());
	}
}
