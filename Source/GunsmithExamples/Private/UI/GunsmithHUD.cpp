// Copyright Kraken Party Limited. All Rights Reserved.


#include "UI/GunsmithHUD.h"

#include "EnhancedInputSubsystems.h"
#include "GSGameplayLibrary.h"
#include "Weapon/GSShootingComponent.h"
#include "Blueprint/UserWidget.h"
#include "UI/GunsmithHUDWidget.h"
#include "GunsmithMoverCharacter.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "Game/GunsmithPlayerController.h"
#include "Health/GSHealthComponent.h"
#include "UI/GunsmithPauseMenuWidget.h"
#include "Widgets/SViewport.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_InputMode_UI, "EnhancedInput.Modes.UI")

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
			PauseMenuWidget->Hide();
		}
	}

	if (PlayerOwner)
	{
		PlayerOwner->OnPossessedPawnChanged.AddUniqueDynamic(this, &AGunsmithHUD::OnPossessedPawnChanged);
		OnPossessedPawnChanged(nullptr, PlayerOwner->GetPawn());

		if (AGunsmithPlayerController* GunsmithController = Cast<AGunsmithPlayerController>(PlayerOwner))
		{
			GunsmithController->OnDeviceChanged.AddDynamic(this, &AGunsmithHUD::OnDeviceChanged);
		}
	}

	if (FSlateApplication::IsInitialized())
	{
		FocusChangedHandle = FSlateApplication::Get().OnFocusChanging().AddUObject(this, &AGunsmithHUD::OnFocusChanged);
	}
}

void AGunsmithHUD::BeginDestroy()
{
	Super::BeginDestroy();

	if (FocusChangedHandle.IsValid() && FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().OnFocusChanging().Remove(FocusChangedHandle);
		FocusChangedHandle.Reset();
	}
}

UWidget* AGunsmithHUD::SetPaused(bool bPaused) const
{
	if (bPaused)
	{
		if (PauseMenuWidget)
		{
			PauseMenuWidget->Activate();
			return PauseMenuWidget->GetDesiredFocusWidget();
		}
	}
	else
	{
		TArray<TObjectPtr<UGunsmithActivatableWidget>> StackCopy = TArray(WidgetStack);
		for (int32 i = StackCopy.Num() - 1; i >= 0; i--)
		{
			UGunsmithActivatableWidget* ActivatableWidget = StackCopy[i];

			if (ActivatableWidget)
			{
				ActivatableWidget->Deactivate();
			}
		}
	}

	return nullptr;
}

void AGunsmithHUD::AddWidgetToStack(UGunsmithActivatableWidget* Widget)
{
	ensureAlways(!WidgetStack.Contains(Widget));

	WidgetStack.Emplace(Widget);

	if (APlayerController* Controller = GetOwningPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = Controller->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			const FGameplayTag& NewMode = TAG_InputMode_UI;
			InputSubsystem->SetInputMode(FGameplayTagContainer(NewMode));
		}
		
		FInputModeGameAndUI UIMode;

		UWidget* RootWidgetToFocus = Widget->GetFocusWidget();
		if (RootWidgetToFocus)
		{
			LastFocusedWidget = RootWidgetToFocus->GetCachedWidget();
			UIMode.SetWidgetToFocus(LastFocusedWidget);
		}

		Controller->SetInputMode(UIMode);
	}
}

void AGunsmithHUD::RemoveWidgetFromStack(UGunsmithActivatableWidget* Widget)
{
	WidgetStack.Remove(Widget);

	const int32 RemainingWidgets = WidgetStack.Num();
	if (RemainingWidgets > 0)
	{
		WidgetStack[RemainingWidgets - 1]->Show(true);
	}
	else
	{
		if (APlayerController* Controller = GetOwningPlayerController())
		{
			if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = Controller->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				const FGameplayTag& NewMode = UE::EnhancedInput::InputMode_Default;
				InputSubsystem->SetInputMode(FGameplayTagContainer(NewMode));
			}
		
			FInputModeGameOnly GameMode;
			Controller->SetInputMode(GameMode);
		}
	}
}

void AGunsmithHUD::DeactivateTopWidget()
{
	const int32 RemainingWidgets = WidgetStack.Num();
	if (RemainingWidgets > 0)
	{
		UGunsmithActivatableWidget* TopWidget = WidgetStack[RemainingWidgets - 1];

		if (TopWidget->GetCanDeactivateWithBackButton())
		{
			TopWidget->Deactivate();
		}
	}
}

void AGunsmithHUD::HideWidgetsInStack()
{
	for (UGunsmithActivatableWidget* Widget : WidgetStack)
	{
		if (Widget)
		{
			Widget->Hide();
		}
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

void AGunsmithHUD::OnDeviceChanged(bool bIsGamepad)
{
	if (bIsGamepad)
	{
		RefreshLastWidgetFocus();
	}
	else if (WidgetStack.Num() == 0)
	{
		if (UGameViewportClient* Viewport = GetWorld()->GetGameViewport())
		{
			if (TSharedPtr<SViewport> ViewportWidget = Viewport->GetGameViewportWidget())
			{
				SetWidgetFocus(ViewportWidget);
			}
		}
	}
}

void AGunsmithHUD::OnFocusChanged(const FFocusEvent& Event, const FWeakWidgetPath& OldPath,
                                  const TSharedPtr<SWidget>& OldWidget, const FWidgetPath& NewPath, const TSharedPtr<SWidget>& NewWidget)
{
	bool bIsNewWidgetAViewport = false;
	if (NewWidget.IsValid())
	{
		bIsNewWidgetAViewport = NewWidget->GetWidgetClass().GetWidgetType() == SViewport::StaticWidgetClass().GetWidgetType();
		if (!bIsNewWidgetAViewport && NewWidget->GetWidgetClass().GetWidgetType() != SWindow::StaticWidgetClass().GetWidgetType())
		{
			LastFocusedWidget = NewWidget;
		}
	}

	bool bIsUsingGamepad = false;

	if (AGunsmithPlayerController* GunsmithController = Cast<AGunsmithPlayerController>(GetOwningPlayerController()))
	{
		bIsUsingGamepad = GunsmithController->WasLastUsingGamepad();
	}

	if (!bIsUsingGamepad)
	{
		return;
	}
	
	// Return the focus to the last focused widget if the viewport is focused on
	if (bIsNewWidgetAViewport && LastFocusedWidget)
	{
		RefreshLastWidgetFocus();
		
		return;
	}
	
	// Force hover when focus is set to the target
	if (OldWidget)
	{
		FPointerEvent PointerEvent;
		OldWidget->OnMouseLeave(PointerEvent);
	}

	if (NewWidget)
	{
		FGeometry Geometry;
		FPointerEvent PointerEvent;
		NewWidget->OnMouseEnter(Geometry, PointerEvent);
	}
}

void AGunsmithHUD::RefreshLastWidgetFocus()
{
	if (LastFocusedWidget)
	{
		SetWidgetFocus(LastFocusedWidget);
	}
}

void AGunsmithHUD::SetWidgetFocus(const TSharedPtr<SWidget>& Widget) const
{
	if (APlayerController* PlayerController = GetOwningPlayerController())
	{
		if ( ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer() )
		{
			FSlateApplication::Get().SetKeyboardFocus(Widget);
			
			TOptional<int32> UserIndex = FSlateApplication::Get().GetUserIndexForController(LocalPlayer->GetControllerId());
			if (UserIndex.IsSet())
			{
				FReply& DelayedSlateOperations = LocalPlayer->GetSlateOperations();
				if (FSlateApplication::Get().SetUserFocus(UserIndex.GetValue(), Widget))
				{
					DelayedSlateOperations.CancelFocusRequest();
				}
				else
				{
					DelayedSlateOperations.SetUserFocus(Widget.ToSharedRef());
				}
			}
		}
	}
}
