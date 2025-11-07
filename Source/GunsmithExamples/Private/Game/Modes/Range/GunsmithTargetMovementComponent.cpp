// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/Modes/Range/GunsmithTargetMovementComponent.h"

#include "NetworkPredictionModelDef.h"
#include "NetworkPredictionModelDefRegistry.h"
#include "NetworkPredictionTrace.h"
#include "NetworkPredictionProxyInit.h"
#include "World/GSWorldStateSubsystem.h"

class FGSTargetMovementNetworkModelDef : public FNetworkPredictionModelDef
{
public:

	NP_MODEL_BODY();

	using Simulation = FGSTargetMovementNetworkSimulation;
	using StateTypes = TGSTargetMovementNetworkSimulationBufferTypes;
	using Driver = UGunsmithTargetMovementComponent;

	static const TCHAR* GetName() { return TEXT("TargetMovementNetSim"); }
	static constexpr int32 GetSortPriority() { return static_cast<int32>(ENetworkPredictionSortPriority::KinematicMovers); }
};

NP_MODEL_REGISTER(FGSTargetMovementNetworkModelDef);

void FGSTargetMovementInputCmd::NetSerialize(const FNetSerializeParams& P)
{
	P.Ar.SerializeBits(&bIsActive, 1);
	P.Ar << ActiveHeight;
}

void FGSTargetMovementSyncState::NetSerialize(const FNetSerializeParams& P)
{
	P.Ar << ActivePercentage;
	P.Ar << ActiveHeight;
}

bool FGSTargetMovementSyncState::ShouldReconcile(const FGSTargetMovementSyncState& AuthorityState) const
{
	UE_NP_TRACE_RECONCILE(ActivePercentage != AuthorityState.ActivePercentage, "ActivePercentage:");
	UE_NP_TRACE_RECONCILE(ActiveHeight != AuthorityState.ActiveHeight, "ActivePercentage:");

	return false;
}

void FGSTargetMovementSyncState::ToString(FAnsiStringBuilderBase& Out) const
{
	Out.Appendf("ActivePercentage: %f\n", ActivePercentage);
	Out.Appendf("ActiveHeight: %f\n", ActiveHeight);
}

void FGSTargetMovementSyncState::Interpolate(const FGSTargetMovementSyncState* From,
                                             const FGSTargetMovementSyncState* To, float PCT)
{
	ActivePercentage = FMath::Lerp(From->ActivePercentage, To->ActivePercentage, PCT);
	ActiveHeight = FMath::Lerp(From->ActiveHeight, To->ActiveHeight, PCT);
}

void FGSTargetMovementAuxState::NetSerialize(const FNetSerializeParams& P)
{
	P.Ar.SerializeBits(&bIsActive, 1);
}

bool FGSTargetMovementAuxState::ShouldReconcile(const FGSTargetMovementAuxState& Authority) const
{
	UE_NP_TRACE_RECONCILE(bIsActive != Authority.bIsActive, "bIsActive:");

	return false;
}

void FGSTargetMovementAuxState::Interpolate(const FGSTargetMovementAuxState* From, const FGSTargetMovementAuxState* To,
	float PCT)
{
	bIsActive = To->bIsActive;
}

void FGSTargetMovementNetworkSimulation::SimulationTick(const FNetSimTimeStep& TimeStep,
                                                        const TNetSimInput<TGSTargetMovementNetworkSimulationBufferTypes>& Input,
                                                        const TNetSimOutput<TGSTargetMovementNetworkSimulationBufferTypes>& Output) const
{
	Output.Aux.Get()->bIsActive = Input.Cmd->bIsActive;
	Output.Sync->ActiveHeight = Input.Cmd->ActiveHeight;
	
	if (MovementComponent.IsValid())
	{
		const float DeltaTime = TimeStep.StepMS / 1000.0f;
		const float MoveDelta = (Output.Aux.Get()->bIsActive ? 1 : -1) * DeltaTime * MovementComponent->AnimateSpeed;
		Output.Sync->ActivePercentage = FMath::Clamp(Input.Sync->ActivePercentage + MoveDelta, 0.0f, 1.0f);
	}
}

UGunsmithTargetMovementComponent::UGunsmithTargetMovementComponent()
{
	bWantsInitializeComponent = true;
	bAutoActivate = true;
	SetIsReplicatedByDefault(true);
}

void UGunsmithTargetMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetOwner()->GetActorLocation();
}

void UGunsmithTargetMovementComponent::InitializeNetworkPredictionProxy()
{
	TargetMovementNetworkSim = MakeUnique<FGSTargetMovementNetworkSimulation>();
	NetworkPredictionProxy.Init<FGSTargetMovementNetworkModelDef>(GetWorld(), GetReplicationProxies(), TargetMovementNetworkSim.Get(), this);

	TargetMovementNetworkSim->MovementComponent = this;
}

void UGunsmithTargetMovementComponent::SetTargetActive(bool bActive)
{
	bIsActive = bActive;

	if (bIsActive)
	{
		ActiveHeight = FMath::RandRange(MinHeight, MaxHeight);
	}
}

bool UGunsmithTargetMovementComponent::GetIsActive() const
{
	if (const FGSTargetMovementAuxState* AuxState = NetworkPredictionProxy.ReadAuxState<FGSTargetMovementAuxState>())
	{
		return AuxState->bIsActive;
	}

	return false;
}

void UGunsmithTargetMovementComponent::InitializeSimulationState(FGSTargetMovementSyncState* Sync,
                                                                 FGSTargetMovementAuxState* Aux)
{
}

void UGunsmithTargetMovementComponent::ProduceInput(const int32 DeltaTimeMS, FGSTargetMovementInputCmd* Cmd)
{
	Cmd->bIsActive = bIsActive;
	Cmd->ActiveHeight = ActiveHeight;
}

void UGunsmithTargetMovementComponent::FinalizeFrame(const FGSTargetMovementSyncState* Sync,
	const FGSTargetMovementAuxState* Aux)
{
	const FVector NewLocation = StartLocation + FVector(0.0f, 0.0f, FMath::Lerp(0.0f, Sync->ActiveHeight, Sync->ActivePercentage));
	GetOwner()->SetActorLocation(NewLocation);
}
