// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/GSCharacter.h"
#include "MoverSimulationTypes.h"
#include "Netcode/GSRollbackInterface.h"
#include "Weapon/Target/GSDamageTargetInterface.h"
#include "GunsmithMoverCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
class UGSMoverComponent;
class UGSRollbackComponent;
class USphereComponent;
class UAnimMontage;
struct FInputActionValue;
struct FGSProjectileFrameState;

/*
 *	A struct containing parameters that affect a characters input
 */
USTRUCT()
struct GUNSMITHEXAMPLES_API FGSCharacterInputConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Input")
	FVector2D LookInputSpeed = FVector2D(1.0f, 1.0f);

	UPROPERTY(EditAnywhere, meta=(ClampMin="-90.0"), Category="Input")
	float InputPitchMin = -90.0f;

	UPROPERTY(EditAnywhere, meta=(ClampMax="90.0"), Category="Input")
	float InputPitchMax = 90.0f;
};

/*
 *	An example of how Gunsmith can be implemented on a playable character. Some of the Mover logic is copied from AMoverExamplesCharacter
 *	This is intentionally not using best practises in favour of being more readable
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithMoverCharacter : public AGSCharacter, public IMoverInputProducerInterface, public IGSDamageTargetInterface, public IGSRollbackInterface
{
	GENERATED_BODY()

public:
	AGunsmithMoverCharacter(const FObjectInitializer& ObjectInitializer);

	// APawn Begin
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual FVector GetVelocity() const override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void NotifyRestarted() override;
	virtual void OnRep_Controller() override;
	virtual FRotator GetBaseAimRotation() const override;
	// APawn End

	// IGSRollbackInterface Begin
	virtual UGSRollbackComponent* GetRollbackComponent_Implementation() const override { return RollbackComponent; }
	// IGSRollbackInterface End

	// IGSDamageTargetInterface Begin
	virtual float GetDamageMultiplierForHitComponent_Implementation(const UPrimitiveComponent* Component) const override;
	virtual UGSHealthComponent* GetHealthComponent_Implementation() const override { return HealthComponent; }
	virtual bool ShouldBlockEmitterOutputTrace_Implementation(const UGSEmitterOutputDataAsset* EmitterOutputData, const FHitResult& HitResult) const override { return true; }
	virtual bool CanEmitterOutputBounceOff_Implementation(const UGSEmitterOutputDataAsset* EmitterOutputData) const override { return false; }
	// IGSDamageTargetInterface End

	/*** Mover ***/
	
	// Accessor for the actor's movement component
	UFUNCTION(BlueprintCallable, Category = "Mover")
	virtual UGSMoverComponent* GetMoverComponent() const { return CharacterMotionComponent; }

	// Request the character starts moving with an intended directional magnitude. A length of 1 indicates maximum acceleration.
	UFUNCTION(BlueprintCallable, Category = "MoverExamples")
	virtual void RequestMoveByIntent(const FVector& DesiredIntent) { CachedMoveInputIntent = DesiredIntent; }

	// Request the character starts moving with a desired velocity. This will be used in lieu of any other input.
	UFUNCTION(BlueprintCallable, Category = "MoverExamples")
	virtual void RequestMoveByVelocity(const FVector& DesiredVelocity) { CachedMoveInputVelocity = DesiredVelocity; }

	/*** Inputs ***/

	// Called on mouse down to enable the shooting component
	void BeginShooting();
	// Called on mouse up to enable the shooting component
	void EndShooting();

	// Called on RMB down to enter the Aim Down Sites state
	void BeginADS();
	// Called on RMB up to leave the Aim Down Sites state
	void EndADS();

	// Called when the shooting component thinks the local players look input has changed (e.g recoil kickback has increased)
	void ApplyLookInput(const FVector2D& LookInputs);

protected:
	// IMoverInputProducerInterface Begin
	// Entry point for input production. Do not override. To extend in derived character types, override OnProduceInput for native types or implement "Produce Input" blueprint event
	virtual void ProduceInput_Implementation(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult) override;

	// Override this function in native class to author input for the next simulation frame. Consider also calling Super method.
	virtual void OnProduceMoverInput(float DeltaMs, FMoverInputCmdContext& OutInputCmd);
	// IMoverInputProducerInterface End

	// AGSCharacter Begin
	virtual void OnProduceShootingInput(float DeltaMs, FGSShootingInputState& InputCmd) override;
	virtual void OnADSStateChanged(bool bADSEnabled) override;
	// AGSCharacter End

	// Save the initial actor rotation into the Player Controllers ControlRotation
	void SaveInitialActorRotation();

	/*** Collision ***/

	// If specified, damage dealt to a specific collider created from a bone will be multiplied by this amount
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
	TMap<FName, float> BoneDamageMultipliers;

	// A list of all bones that should be used to generate rollback colliders
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
	TArray<FName> RollbackBones;

	// Which collision profile should be used for rollback colliders
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
	FName RollbackCollisionProfileName;

	// Which collision channel object type should be used for rollback colliders
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
	TEnumAsByte<ECollisionChannel> RollbackCollisionObjectType = ECC_Pawn;
	
	/** Input Actions */
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> InputMappingContext = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveInputAction = nullptr;
   
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookInputMouseAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookInputGamepadAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpInputAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> FireAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ADSAction = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ReloadAction = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> NextWeaponAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> PreviousWeaponAction = nullptr;
	
	// A list of input actions which when activated, will activate the related equipment slot
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TArray<TObjectPtr<UInputAction>> EquipmentSlotActions;

	// The current look rotation used by the character
	UPROPERTY(BlueprintReadOnly, Category = "Input")
	FRotator LookRotation = FRotator::ZeroRotator;

	// A struct containing input setup values
	UPROPERTY(EditAnywhere, Category = "Input")
	FGSCharacterInputConfig InputConfig;

	// If true, the shooting input is down and the value will be pushed to NPP
	UPROPERTY(BlueprintReadOnly, Category = "Input")
	bool bIsShootingInputDown = false;
	// If true, the shooting input has recently been pressed and will catch any cases where getting the shooting input doesn't tick while it is down
	bool bHasJustPressedShootingInput = false;

	// If true, the ADS input is down and the value will be pushed to NPP
	UPROPERTY(BlueprintReadOnly, Category = "Input")
	bool bIsADSInputDown = false;

	// If true, the reload input is down and the value will be pushed to NPP
	UPROPERTY(BlueprintReadOnly, Category = "Input")
	bool bIsReloadTriggered = false;

	// Trigger a shooting component reload
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void TriggerReload();
	
	// Applies the look inputs from the mouse to the look rotation
	virtual FRotator ApplyRotation(const FRotator& InLookRotation, const FVector2D& LookInputs) const;

	int32 CurrentWeaponSlot = 0;

	/*** Camera ***/
	
	// FOV settings should be implemented in the PlayerCameraManager but for sake of simplicity we're doing it here
	// The FOV level to set when not aiming down sights
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float InitialFOVValue = 90.0f;

	// The FOV level to set when aiming down sights
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float ADSFOVValue = 80.0f;

	// How long should it take to blend between ADS FOV values
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float FOVChangeTime = 0.2f;

	/*** Components ***/
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rollback")
	TObjectPtr<UGSRollbackComponent> RollbackComponent = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	TObjectPtr<UGSMoverComponent> CharacterMotionComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	TObjectPtr<UGSHealthComponent> HealthComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArmComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> CameraComponent;

	/*** Animation ***/

	// A list of death montages that play in random when health reaches 0
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	TArray<TObjectPtr<UAnimMontage>> DeathMontages;

	// Which bone should the ragdoll impulse be applied to on death
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	FName RagdollImpulseBone;

	// How strong should the ragdoll impulse be
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	float RagdollImpulseStrength = 500.0f;

	// A list of hit reacts to play one at random when hit on the front
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TArray<TObjectPtr<UAnimMontage>> HitReactsFront;

	// A list of hit reacts to play one at random when hit on the right
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TArray<TObjectPtr<UAnimMontage>> HitReactsRight;

	// A list of hit reacts to play one at random when hit on the back
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TArray<TObjectPtr<UAnimMontage>> HitReactsBack;

	// A list of hit reacts to play one at random when hit on the left
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TArray<TObjectPtr<UAnimMontage>> HitReactsLeft;

	// How long to wait before spawning a new character after death
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeathTimeBeforeDestroy = 3.0f;

	// Play a hit react based on which direction the character was hit from
	void PlayHitReact(int32 Seed, const FVector& HitNormal) const;

	/*** Debug ***/

	UPROPERTY(VisibleAnywhere, Category = "Debug")
	float TimeToDebugMoveInOneDirection = 1.0f;

	UPROPERTY(VisibleAnywhere, Category = "Debug")
	float TimeBetweenDebugJump = 1.6f;

	virtual void DrawCurrentLocationDebug(bool bRoundToFullFrame, const FName& LogReference) override;
	
#if !UE_BUILD_SHIPPING
public:
	static TArray<uint32> EnabledDebugMovers;
	
	// Enables debug movement to move a character repeatedly left and right
	void EnableDebugMovement(bool bDebugMove);

private:
	bool bIsDebugMoving = false;
	float TimeDebugMoving = 0.0f;
	bool bDebugMovingRight = false;

	float TimeSinceDebugJump = 0.0f;
	bool bIsDebugJumping = false;

	int32 AutoShootProjectileCount = 0;
	int32 AutoShootProjectileHitCount = 0;
	
	// Tick the debug movement
	void UpdateDebugMovement(float DeltaTime);
#endif

	UFUNCTION()
	void OnAutoShootProjectileHitTarget(int32 Frame, const FHitResult& Hit, UGSProjectileState* ProjectileState, const FGSProjectileFrameState& FrameState);
	UFUNCTION()
	void OnAutoShootProjectileDestroyed(int32 Frame, bool bHitTarget, const FHitResult& HitResult, UGSProjectileState* ProjectileState, const FGSProjectileFrameState& CurrentFrameData);

	UFUNCTION()
	void OnProjectileCreated(UGSProjectileState* ProjectileState);

	/*** Inputs ***/
	
	FVector LastAffirmativeMoveInput = FVector::ZeroVector;	// Movement input (intent or velocity) the last time we had one that wasn't zero

	FVector CachedMoveInputIntent = FVector::ZeroVector;
	FVector CachedMoveInputVelocity = FVector::ZeroVector;
	FVector LastMoveInputIntent = FVector::ZeroVector;

	FRotator CachedTurnInput = FRotator::ZeroRotator;

	bool bIsJumpJustPressed = false;
	bool bIsJumpPressed = false;
	bool bIsFlyingActive = false;
	
	void OnMoveTriggered(const FInputActionValue& Value);
	void OnMoveCompleted(const FInputActionValue& Value);
	void OnLookTriggered(const FInputActionValue& Value);
	void OnJumpStarted(const FInputActionValue& Value);
	void OnJumpReleased(const FInputActionValue& Value);
	void OnReloadPressed(const FInputActionValue& Value);
	void OnNextWeaponPressed(const FInputActionValue& Value);
	void OnPreviousWeaponPressed(const FInputActionValue& Value);
	void OnEquipmentSlotPressed(const FInputActionValue& Value, int32 Slot);

	// Change the active weapon to the slot in input direction
	void ChangeWeapon(int32 Direction);

	virtual FRotator GetAuthoritativeAimRotation() const;

	/*** Death ***/
	
	bool bIsDead = false;

	UFUNCTION()
	void OnDeath(UGSHealthComponent* AffectedHealthComponent, const FGSDamageRecord& DamageRecord, bool bIsPredicted);

	UFUNCTION()
	void OnDamageTaken(UGSHealthComponent* AffectedHealthComponent, const FGSDamageRecord& DamageRecord, bool bIsPredicted);

	/*** FOV ***/
	
	bool bIsFOVIncreasing = false;
	float CurrentADSFOVAlpha = 0.0f;
};
