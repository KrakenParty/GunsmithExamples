// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NetworkPredictionComponent.h"
#include "NetworkPredictionStateTypes.h"
#include "Netcode/GSRollbackNetProxy.h"
#include "GunsmithTargetMovementComponent.generated.h"

class UGunsmithTargetMovementComponent;

struct FGSTargetMovementInputCmd
{
	bool bIsActive = false;
	float ActiveHeight = 0.0f;
	
	void NetSerialize(const FNetSerializeParams& P);
	void ToString(FAnsiStringBuilderBase& Out) const { }
};

USTRUCT(BlueprintType)
struct FGSTargetMovementSyncState
{
	GENERATED_BODY()
	
	// The current location as a value between 0-1
	UPROPERTY(BlueprintReadWrite, Category = "Gunsmith")
	float ActivePercentage = 0.0f;

	// How high the target will be when active
	UPROPERTY(BlueprintReadWrite, Category = "Gunsmith")
	float ActiveHeight = 0.0f;
	
	void NetSerialize(const FNetSerializeParams& P);
	bool ShouldReconcile(const FGSTargetMovementSyncState& AuthorityState) const;
	void ToString(FAnsiStringBuilderBase& Out) const;
	void Interpolate(const FGSTargetMovementSyncState* From, const FGSTargetMovementSyncState* To, float PCT);
};

USTRUCT(BlueprintType)
struct FGSTargetMovementAuxState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Gunsmith")
	bool bIsActive = false;
	
	void NetSerialize(const FNetSerializeParams& P);
	bool ShouldReconcile(const FGSTargetMovementAuxState& Authority) const;
	void ToString(FAnsiStringBuilderBase& Out) const { }
	void Interpolate(const FGSTargetMovementAuxState* From, const FGSTargetMovementAuxState* To, float PCT);
};

using TGSTargetMovementNetworkSimulationBufferTypes = TNetworkPredictionStateTypes<FGSTargetMovementInputCmd, FGSTargetMovementSyncState, FGSTargetMovementAuxState>;

struct FNetSimTimeStep;

class FGSTargetMovementNetworkSimulation
{
public:
	void SimulationTick(const FNetSimTimeStep& TimeStep, const TNetSimInput<TGSTargetMovementNetworkSimulationBufferTypes>& Input, const TNetSimOutput<TGSTargetMovementNetworkSimulationBufferTypes>& Output) const;

	TWeakObjectPtr<UGunsmithTargetMovementComponent> MovementComponent;
};

/**
 *	A network prediction component used for targets to ensure that they are synced and can be rolled back correctly
 */
UCLASS()
class GUNSMITHEXAMPLES_API UGunsmithTargetMovementComponent : public UNetworkPredictionComponent
{
	GENERATED_BODY()

public:
	UGunsmithTargetMovementComponent();

	virtual void BeginPlay() override;

	// UNetworkPredictionComponent Begin
	virtual void InitializeNetworkPredictionProxy() override;
	// UNetworkPredictionComponent End

	void SetTargetActive(bool bActive);
	bool GetIsActive() const;
	
	/*** Simulation ***/
	
	virtual void InitializeSimulationState(FGSTargetMovementSyncState* Sync, FGSTargetMovementAuxState* Aux);
	virtual void ProduceInput(const int32 DeltaTimeMS, FGSTargetMovementInputCmd* Cmd);
	virtual void FinalizeFrame(const FGSTargetMovementSyncState* Sync, const FGSTargetMovementAuxState* Aux);

	/*** Setup ***/

	// Min height of the target when active
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	float MinHeight = 100.0f;

	// Max height of the target when active
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	float MaxHeight = 220.0f;

	// How fast to animate between states
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	float AnimateSpeed = 1.0f;

protected:
	TUniquePtr<FGSTargetMovementNetworkSimulation> TargetMovementNetworkSim;
	
	bool bIsActive = false;
	float ActiveHeight = 0.0f;
	FVector StartLocation = FVector::ZeroVector;
};