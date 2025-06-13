// Copyright Kraken Party Limited. All Rights Reserved.

#include "UI/GunsmithMultiplayerHUD.h"

#include "Blueprint/UserWidget.h"
#include "UI/GunsmithMultiplayerHUDWidget.h"

void AGunsmithMultiplayerHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (MultiplayerModeHUDWidgetType)
	{
		MultiplayerHUDWidget = CreateWidget<UGunsmithMultiplayerHUDWidget>(PlayerOwner, MultiplayerModeHUDWidgetType, TEXT("MultiplayerHUD"));

		if (MultiplayerHUDWidget)
		{
			MultiplayerHUDWidget->AddToViewport();
		}
	}
}
