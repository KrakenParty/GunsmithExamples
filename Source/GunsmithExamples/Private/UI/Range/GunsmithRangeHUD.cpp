// Copyright Kraken Party Limited. All Rights Reserved.

#include "UI/Range/GunsmithRangeHUD.h"

#include "Blueprint/UserWidget.h"
#include "UI/Range/GunsmithRangeHUDWidget.h"

void AGunsmithRangeHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (RangeHUDWidgetType)
	{
		RangeHUDWidget = CreateWidget<UGunsmithRangeHUDWidget>(PlayerOwner, RangeHUDWidgetType, TEXT("RangeHUD"));

		if (RangeHUDWidget)
		{
			RangeHUDWidget->AddToViewport();
		}
	}
}
