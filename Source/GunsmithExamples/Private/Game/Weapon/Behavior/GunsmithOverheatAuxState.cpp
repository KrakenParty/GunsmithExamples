// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/Weapon/Behavior/GunsmithOverheatAuxState.h"

#include "NetworkPredictionTrace.h"

FMoverDataStructBase* FGunsmithOverheatAuxState::Clone() const
{
	FGunsmithOverheatAuxState* CopyPtr = new FGunsmithOverheatAuxState(*this);
	return CopyPtr;
}

bool FGunsmithOverheatAuxState::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	const bool bIsSaving = Ar.IsSaving();
	SerializeOptionalValue(bIsSaving, Ar, TimeUntilCoolDown, 0.0f);

	return bOutSuccess;
}

bool FGunsmithOverheatAuxState::ShouldReconcile(const FMoverDataStructBase& AuthorityState) const
{
	const FGunsmithOverheatAuxState* AuthorityAuxState = static_cast<const FGunsmithOverheatAuxState*>(&AuthorityState);
	
	UE_NP_TRACE_RECONCILE(!FMath::IsNearlyEqual(TimeUntilCoolDown, AuthorityAuxState->TimeUntilCoolDown), "TimeUntilCoolDown: ");
	return false;
}

void FGunsmithOverheatAuxState::ToString(FAnsiStringBuilderBase& Out) const
{
	FMoverDataStructBase::ToString(Out);

	Out.Appendf("TimeUntilCoolDown: %f\n", TimeUntilCoolDown);
}

void FGunsmithOverheatAuxState::Interpolate(const FMoverDataStructBase& From, const FMoverDataStructBase& To, float Pct)
{
	const FGunsmithOverheatAuxState* ToState = static_cast<const FGunsmithOverheatAuxState*>(&To);

	TimeUntilCoolDown = ToState->TimeUntilCoolDown;
}
