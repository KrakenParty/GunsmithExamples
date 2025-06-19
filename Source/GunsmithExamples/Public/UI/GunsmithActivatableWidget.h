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
	
	UFUNCTION(BlueprintCallable, Category="UI")
	UWidget* GetFocusWidget() const;

protected:
	UFUNCTION(BlueprintNativeEvent, Category="UI")
	UWidget* GetRootWidget();
};
