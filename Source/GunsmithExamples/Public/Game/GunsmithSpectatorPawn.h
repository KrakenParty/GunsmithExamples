// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GunsmithMoverCharacter.h"
#include "GameFramework/SpectatorPawn.h"
#include "GunsmithSpectatorPawn.generated.h"

class UInputAction;
class UGunsmithSpringArmComponent;
class UCameraComponent;
struct FInputActionValue;

/**
 *	A basic spectator pawn that follows other active players
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithSpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()

public:
	AGunsmithSpectatorPawn();

	// ASpectatorPawn Begin
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* InInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;

	virtual void Tick(float DeltaSeconds) override;
	// ASpectatorPawn End

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookInputMouseAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SwitchViewTargetAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ScrollViewAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta=(ClampMin="0"))
	float MinScrollDistance = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta=(ClampMin="0"))
	float MaxScrollDistance = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta=(ClampMin="0"))
	float ScrollSpeed = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta=(ClampMin="0"))
	float ScrollSmoothTime = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta=(ClampMin="0"))
	float ScrollAcceleration = 20.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UGunsmithSpringArmComponent> SpringArmComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> CameraComponent;
	
	TWeakObjectPtr<APawn> SpectatedPawn;

	float DesiredTargetArmLength = 0.0f;
	float ArmLengthVelocity = 0.0f;
	int32 ScrollDirection = 1;

	// A struct containing input setup values
	UPROPERTY(EditAnywhere, Category = "Input")
	FGSCharacterInputConfig InputConfig;

	void SwitchSpectatedPlayer(int32 Direction);
	
	// Find the next player that we can spectate
	APawn* GetNextSpectatablePawn(int32 Dir) const;

	APawn* GetNextSpectatableDemoPawn(int32 Dir) const;

	// Returns true if we can spectate the target
	static bool CanSpectate(AController* Viewer, const APlayerState* ViewTarget);

	void OnLookTriggered(const FInputActionValue& Value);
	void OnSwitchViewTargetStarted(const FInputActionValue& Value);
	void OnScrollViewTriggered(const FInputActionValue& Value);

	UFUNCTION()
	void OnSpectatedPawnDestroyed(AActor* DestroyedActor);
};
