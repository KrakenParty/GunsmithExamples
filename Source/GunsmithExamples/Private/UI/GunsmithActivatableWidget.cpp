// Copyright Kraken Party Limited. All Rights Reserved.

#include "UI/GunsmithActivatableWidget.h"

#include "GunsmithBlueprintFunctionLibrary.h"

void UGunsmithActivatableWidget::Activate()
{
	// TODO Set up proper layers at some point and deactivate lower layers which this is active
	GetRootWidget()->SetVisibility(ESlateVisibility::Visible);

	if (APlayerController* Controller = GetOwningPlayer())
	{
		if (UWidget* FocusWidget = GetFocusWidget())
		{
			FocusWidget->SetUserFocus(Controller);
		}
	}
}

void UGunsmithActivatableWidget::Deactivate()
{
	GetRootWidget()->SetVisibility(ESlateVisibility::Collapsed);
}

UWidget* UGunsmithActivatableWidget::GetFocusWidget() const
{
	UWidget* FocusWidget = GetDesiredFocusWidget();
	UWidget* RootWidget = UGunsmithBlueprintFunctionLibrary::GetRootFocusWidget(FocusWidget);
	return RootWidget;
}

UWidget* UGunsmithActivatableWidget::GetRootWidget_Implementation()
{
	return this;
}
