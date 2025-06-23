// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GunsmithActivatableWidget.generated.h"

/**
 *	A base class widget to be used as a replacement for CommonUI
 */
UCLASS()
class GUNSMITHEXAMPLES_API UGunsmithActivatableWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// Show this widget and set the focus
	UFUNCTION(BlueprintCallable, Category="UI")
	void Activate();

	// Deactivate this widget and show any UI underneath
	UFUNCTION(BlueprintCallable, Category="UI")
	void Deactivate();

	// Sets the widget to be visible but doesn't register it with the HUD system
	UFUNCTION(BlueprintCallable, Category="UI")
	void Show(bool bSetFocus);

	// Sets the widget to be collapsed but doesn't unregister it with the HUD system
	UFUNCTION(BlueprintCallable, Category="UI")
	void Hide();

	// Return true if this widget is visible and active
	UFUNCTION(BlueprintCallable, Category="UI")
	bool GetIsActive() const { return bIsActive; }

	// Return the reference to the widget that should gain focus
	UFUNCTION(BlueprintCallable, Category="UI")
	UWidget* GetFocusWidget() const;

	bool GetCanDeactivateWithBackButton() const { return bCanDeactivateWithBackButton; }

protected:
	// If true, will close any other open widgets when this is activated
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	bool bCloseOtherWidgetsOnActivate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	bool bCanDeactivateWithBackButton = true;
	
	UFUNCTION(BlueprintNativeEvent, Category="UI")
	UWidget* GetRootWidget();

private:
	bool bIsActive = false;
};
