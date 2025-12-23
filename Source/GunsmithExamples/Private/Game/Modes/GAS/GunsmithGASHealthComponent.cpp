// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/Modes/GAS/GunsmithGASHealthComponent.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "GSGameplayLibrary.h"
#include "Health/GSHealthViewModel.h"
#include "Netcode/GSNetworkLibrary.h"
#include "Weapon/GSShootingComponent.h"
#include "Weapon/Damage/GSGameplayEffectDamageFormat.h"

void UGunsmithGASHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UGSShootingComponent* ShootingComponent = UGSGameplayLibrary::GetShootingComponentFromActor(GetOwner()))
	{
		TWeakObjectPtr<UGunsmithGASHealthComponent> WeakThis(this);
		ShootingComponent->CallOrRegisterAbilityComponentSetupEvent(FGSAbilitySystemSetupDelegate::FDelegate::CreateWeakLambda(this, [WeakThis](UAbilitySystemComponent* AbilitySystemComponent)
		{
			if (!WeakThis.Get())
			{
				return;
			}
			
			if (WeakThis->HealthAttribute.IsValid())
			{
				AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(WeakThis->HealthAttribute).AddUObject(WeakThis.Get(), &UGunsmithGASHealthComponent::OnHealthChanged);

				if (WeakThis->HealthVM)
				{
					WeakThis->HealthVM->SetHealth(AbilitySystemComponent->GetNumericAttribute(WeakThis->HealthAttribute));
				}
			}

			if (WeakThis->MaxHealthAttribute.IsValid())
			{
				AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(WeakThis->MaxHealthAttribute).AddUObject(WeakThis.Get(), &UGunsmithGASHealthComponent::OnMaxHealthChanged);

				if (WeakThis->HealthVM)
				{
					WeakThis->HealthVM->SetMaxHealth(AbilitySystemComponent->GetNumericAttribute(WeakThis->MaxHealthAttribute));
				}
			}
		}));
	}
}

void UGunsmithGASHealthComponent::UpdateHealthViewModel_Implementation(const FGSHealthSyncState& Sync,
                                                                       const FGSHealthAuxState& Aux)
{
	// Prevent health updates from the simulation as it will be driven entirely by GAS
}

void UGunsmithGASHealthComponent::ApplyDamage_Implementation(FGSDamageRequest& Request)
{
	// Prevent damage application as they will be handled by GAS effects
}

void UGunsmithGASHealthComponent::OnHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	// GEModData is only available wherever the GE is applied
	if (UGSNetworkLibrary::HasAuthority(this) && ChangeData.GEModData && ChangeData.GEModData->EvaluatedData.Magnitude < 0.0f)
	{
		const FGameplayEffectSpec& EffectSpec = ChangeData.GEModData->EffectSpec;
		const FGameplayEffectContextHandle& Context = EffectSpec.GetContext();
	
		FGSDamageRecord Record;
		Record.Damage = -ChangeData.GEModData->EvaluatedData.Magnitude;
		Record.Causer = Context.GetEffectCauser();
		Record.HitSourceLocation = Context.GetOrigin();
		Record.bKilledTarget = ChangeData.NewValue <= 0.0f;
		Record.DamageApplicationMode = EGSDamageApplicationMode::None;

		NetMulticast_SendDamageRecordCue(Record, false);

		// Sent a message back to the client to validate the hit
		if (UGSShootingComponent* ShootingComponent = UGSGameplayLibrary::GetShootingComponentFromActor(Record.Causer))
		{
			const float DamageMultiplier = EffectSpec.GetSetByCallerMagnitude(TAG_Weapon_SetByCaller_DamageMultiplier, false, 1.0f);
			ShootingComponent->Client_NotifyHit(this, Record.Damage, DamageMultiplier);
		}
	}
	
	if (HealthVM)
	{
		HealthVM->SetHealth(ChangeData.NewValue);
	}
}

void UGunsmithGASHealthComponent::OnMaxHealthChanged(const FOnAttributeChangeData& ChangeData) const
{
	if (HealthVM)
	{
		HealthVM->SetMaxHealth(ChangeData.NewValue);
	}
}
