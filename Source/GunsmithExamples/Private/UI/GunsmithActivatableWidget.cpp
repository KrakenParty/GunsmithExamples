// Copyright Kraken Party Limited. All Rights Reserved.

#include "UI/GunsmithActivatableWidget.h"

#include "GunsmithBlueprintFunctionLibrary.h"
#include "UI/GunsmithHUD.h"

void UGunsmithActivatableWidget::Activate()
{
	bIsActive = true;
	
	Show(true);

	if (APlayerController* Controller = GetOwningPlayer())
	{
		if (AGunsmithHUD* GunsmithHUD = Controller->GetHUD<AGunsmithHUD>())
		{
			if (bCloseOtherWidgetsOnActivate)
			{
				GunsmithHUD->HideWidgetsInStack();
			}
			
			GunsmithHUD->AddWidgetToStack(this);
		}
	}
}

void UGunsmithActivatableWidget::Deactivate()
{
	bIsActive = false;
	
	Hide();
	
	if (APlayerController* Controller = GetOwningPlayer())
	{
		if (AGunsmithHUD* GunsmithHUD = Controller->GetHUD<AGunsmithHUD>())
		{
			GunsmithHUD->RemoveWidgetFromStack(this);
		}
	}
}

void UGunsmithActivatableWidget::Show(bool bSetFocus)
{	
	GetRootWidget()->SetVisibility(ESlateVisibility::Visible);

	if (bSetFocus)
	{
		if (APlayerController* Controller = GetOwningPlayer())
		{
			if (AGunsmithHUD* GunsmithHUD = Controller->GetHUD<AGunsmithHUD>())
			{
				if (UWidget* FocusWidget = GetFocusWidget())
				{
					GunsmithHUD->SetWidgetFocus(FocusWidget->GetCachedWidget());
				}
			}
		}
	}
}

void UGunsmithActivatableWidget::Hide()
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
