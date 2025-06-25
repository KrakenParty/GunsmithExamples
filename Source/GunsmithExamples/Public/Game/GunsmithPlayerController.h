// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GunsmithPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UWidget;
struct FInputActionValue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGunsmithDeviceChangedDelegate, bool, bIsUsingGamepad);

/*
 *	A class to hold references to all commonly used inputs
 */
UCLASS(Blueprintable)
class UGunsmithCommonInputs : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|UI")
	TObjectPtr<UInputMappingContext> UIInputMappingContext = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|UI")
	TObjectPtr<UInputAction> PauseInputAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|UI")
	TObjectPtr<UInputAction> CancelUIInputAction = nullptr;
};

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
	virtual ASpectatorPawn* SpawnSpectatorPawn() override;
	virtual void SetInitialLocationAndRotation(const FVector& NewLocation, const FRotator& NewRotation) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// APlayerController End

	FGunsmithDeviceChangedDelegate OnDeviceChanged;
	
	// Sets the games paused state - Multiplayer games will not be paused
	UFUNCTION(BlueprintCallable, Category="Gunsmith")
	void SetPaused(bool bPaused);

	UFUNCTION(BlueprintCallable, Category="Gunsmith")
	void SetUIInputMode(bool bEnabled, UWidget* WidgetToFocus = nullptr);

	// Returns true if the last input was from a gamepad
	bool WasLastUsingGamepad() const { return bWasLastUsingGamepad; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TSubclassOf<UGunsmithCommonInputs> CommonInputs = nullptr;

private:
	UPROPERTY(Replicated)
	FRotator SpawnRotation;
	
	bool bWasLastUsingGamepad = true;
	
	void OnPausePressed(const FInputActionValue& Value);
	void OnCancelPressed(const FInputActionValue& Value);

	UFUNCTION()
	void OnHardwareDeviceChanged(const FPlatformUserId UserId, const FInputDeviceId DeviceId);
};
