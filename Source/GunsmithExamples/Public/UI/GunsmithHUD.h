// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "GameFramework/HUD.h"
#include "GunsmithHUD.generated.h"

class UGunsmithHUDWidget;
class UGunsmithPauseMenuWidget;
class UGunsmithActivatableWidget;
class UWidget;

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_InputMode_UI)

/**
 *	The base HUD class for the Gunsmith Samples Project
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithHUD : public AHUD
{
	GENERATED_BODY()

public:
	// AActor Begin
	virtual void PostInitializeComponents() override;
	virtual void BeginDestroy() override;
	// AActor End

	// Get the main Gunsmith HUD widget
	UFUNCTION(BlueprintCallable, Category="Gunsmith")
	UGunsmithHUDWidget* GetHUDWidget() const { return HUDWidget; }

	// Sets the HUD to the paused state. Returns a focus widget if one exists
	UWidget* SetPaused(bool bPaused) const;

	// Widget Management
	void AddWidgetToStack(UGunsmithActivatableWidget* Widget);
	void RemoveWidgetFromStack(UGunsmithActivatableWidget* Widget);

	// Return true if any widgets are active
	bool HasAnyActiveWidgets() const { return WidgetStack.Num() > 0; }
	
	// If a widget is active, deactivate it
	void DeactivateTopWidget();

	// Hides all existing widgets
	void HideWidgetsInStack();

	void SetWidgetFocus(const TSharedPtr<SWidget>& Widget) const;
	
protected:
	// The subclass of HUD widget to spawn
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGunsmithHUDWidget> HUDWidgetType = nullptr;

	// A pointer to the spawned HUD widget
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UGunsmithHUDWidget> HUDWidget = nullptr;

	// The subclass of pause menu widget to spawn
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGunsmithPauseMenuWidget> PauseMenuWidgetType = nullptr;

	// A pointer to the spawned pause menu widget
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UGunsmithPauseMenuWidget> PauseMenuWidget = nullptr;

	UPROPERTY()
	TArray<TObjectPtr<UGunsmithActivatableWidget>> WidgetStack;

	TSharedPtr<SWidget> LastFocusedWidget = nullptr;

	// Bind new events if the possessed pawn changes
	UFUNCTION()
	virtual void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	UFUNCTION()
	void OnDeviceChanged(bool bIsGamepad);
	
	FDelegateHandle FocusChangedHandle;
	
	void OnFocusChanged(const FFocusEvent& Event, const FWeakWidgetPath& OldPath, const TSharedPtr<SWidget>& OldWidget, const FWidgetPath& NewPath, const TSharedPtr<SWidget>& NewWidget);
	void RefreshLastWidgetFocus();
};
