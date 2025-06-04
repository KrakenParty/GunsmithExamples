// Copyright Kraken Party Limited. All Rights Reserved.


#include "UI/GunsmithHUD.h"

#include "GSGameplayLibrary.h"
#include "Weapon/GSShootingComponent.h"
#include "Blueprint/UserWidget.h"
#include "UI/GunsmithHUDWidget.h"
#include "GunsmithMoverCharacter.h"
#include "Health/GSHealthComponent.h"


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

	if (PlayerOwner)
	{
		PlayerOwner->OnPossessedPawnChanged.AddUniqueDynamic(this, &AGunsmithHUD::OnPossessedPawnChanged);
		OnPossessedPawnChanged(nullptr, PlayerOwner->GetPawn());
	}
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
