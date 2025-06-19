// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GunsmithPlayerController.generated.h"

class UInputAction;
class UWidget;
struct FInputActionValue;

/**
 *	A base class player controller for all Gunsmith controllers to inherit from
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// APlayerController Begin
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual bool ShouldShowMouseCursor() const override;
	// APlayerController End

	// Sets the games paused state - Multiplayer games will not be paused
	UFUNCTION(BlueprintCallable, Category="Gunsmith")
	void SetPaused(bool bPaused);

	UFUNCTION(BlueprintCallable, Category="Gunsmith")
	void SetUIInputMode(bool bEnabled, UWidget* WidgetToFocus = nullptr);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> PauseInputAction = nullptr;

private:
	bool bIsPaused = false;
	bool bWasLastUsingGamepad = true;
	
	void OnPausePressed(const FInputActionValue& Value);

	UFUNCTION()
	void OnHardwareDeviceChanged(const FPlatformUserId UserId, const FInputDeviceId DeviceId);
};
