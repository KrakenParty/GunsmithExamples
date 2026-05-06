// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GunsmithPlayerController.generated.h"

class UGSWeaponDataAsset;
class UInputAction;
class UInputMappingContext;
class UWidget;
class ULyraNumberPopComponent_NiagaraText;
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TArray<TObjectPtr<UInputMappingContext>> AdditionalInputMappingContexts;
	
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
	AGunsmithPlayerController();
	
	// APlayerController Begin
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual bool ShouldShowMouseCursor() const override;
	virtual void SetInitialLocationAndRotation(const FVector& NewLocation, const FRotator& NewRotation) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual ASpectatorPawn* SpawnSpectatorPawn() override;
	virtual void OnRep_Pawn() override;
	virtual void SetPawn(APawn* InPawn) override;
	// APlayerController End

	FGunsmithDeviceChangedDelegate OnDeviceChanged;
	
	// Sets the games paused state - Multiplayer games will not be paused
	UFUNCTION(BlueprintCallable, Category="Gunsmith")
	void SetPaused(bool bPaused);

	// Enables or disables the UI input map
	UFUNCTION(BlueprintCallable, Category="Gunsmith")
	void SetUIInputMode(bool bEnabled, UWidget* WidgetToFocus = nullptr);

	// An easily accessible function to restart the pawn on the server. Not ideal for games trying to be secure
	UFUNCTION(BlueprintCallable, Category="Gunsmith")
	void RequestServerRestartPawn();

	// Returns true if the last input was from a gamepad
	bool WasLastUsingGamepad() const { return bWasLastUsingGamepad; }
	
	// A cheat to kill the controllers current pawn
	UFUNCTION(Server, Reliable, Category="Gunsmith")
	void Server_KillPawn();
	
	UFUNCTION(Server, Reliable, Category="Gunsmith")
	void Server_EquipWeapon(UGSWeaponDataAsset* WeaponData);
	
	UFUNCTION(BlueprintCallable, Category="Gunsmith")
	ULyraNumberPopComponent_NiagaraText* GetNumberPopComponent() const { return NumberPopComponent; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TSubclassOf<UGunsmithCommonInputs> CommonInputs = nullptr;
	
	// A component that handles all pop up numbers when damage is dealt
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<ULyraNumberPopComponent_NiagaraText> NumberPopComponent;

	void OnPausePressed(const FInputActionValue& Value);

	void UpdateSpectatorState();

private:
	UPROPERTY(Replicated)
	FRotator SpawnRotation;
	
	bool bWasLastUsingGamepad = true;
	
	void OnCancelPressed(const FInputActionValue& Value);

	UFUNCTION()
	void OnHardwareDeviceChanged(const FPlatformUserId UserId, const FInputDeviceId DeviceId);

	UFUNCTION()
	void OnControlledPawnDeath(UGSHealthComponent* HealthComponent, const FGSDamageRecord& DamageRecord, bool bIsPredicted);
};
