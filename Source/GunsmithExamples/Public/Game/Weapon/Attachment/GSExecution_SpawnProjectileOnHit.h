// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Weapon/GSWeaponAttribute.h"
#include "Weapon/Attachment/GSWeaponAttachmentExecution.h"
#include "GSExecution_SpawnProjectileOnHit.generated.h"

class UGSProjectileDataAsset;
class UGSProjectileState;
struct FGSProjectileFrameState;
struct FHitResult;

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Weapon_Attribute_Attachment_SpawnProjectileOnHit_NumProjectiles)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Weapon_Attribute_Attachment_SpawnProjectileOnHit_SpreadRadius)

/**
 *  An execution which when applied, creates a new projectile when the current projectile hits a target
 */
UCLASS()
class GUNSMITHEXAMPLES_API UGSExecution_SpawnProjectileOnHit : public UGSWeaponAttachmentExecution
{
	GENERATED_BODY()
	
public:
	// UGSWeaponAttachmentExecution Begin
	virtual void Setup(UGSShootingComponent* ShootingComponent, int32 ID) override;
	virtual void Apply(UGSShootingComponent* ShootingComponent, const FGSEquipData& CurrentWeapon, int32 Frame) override;
	virtual void Remove(UGSShootingComponent* ShootingComponent) override;

	virtual UGSEmitterOutputDataAsset* GetAssociatedEmitterOutputDataAsset() const override;
	// UGSWeaponAttachmentExecution End

protected:
	// Which projectile data should be spawned
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Projectile")
	TObjectPtr<UGSProjectileDataAsset> ProjectileToSpawn = nullptr;

	// How many projectiles to spawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta=(ClampMin="1"))
	FGSWeaponAttribute NumProjectiles = FGSWeaponAttribute(TAG_Weapon_Attribute_Attachment_SpawnProjectileOnHit_NumProjectiles, 1);

	// How far can the projectile angle randomly spread from the normal of the hit
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta=(ClampMin="0"))
	FGSWeaponAttribute ProjectileSpreadRadius = FGSWeaponAttribute(TAG_Weapon_Attribute_Attachment_SpawnProjectileOnHit_SpreadRadius, 0.0f);
	
	// Only applies this effect to projectiles with matching tags
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta=(Categories="Weapon.Emitter.Projectile"))
	FGameplayTagContainer ProjectileTagFilter;

	// Only applies this effect to projectiles with matching tags
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	bool bIgnoreSpawningIfCannotBounceOnTarget = true;

	UFUNCTION()
	virtual void OnProjectileSpawned(UGSProjectileState* Projectile);
	UFUNCTION()
	virtual void OnProjectileHitTarget(int32 Frame, const FHitResult& Hit, UGSProjectileState* ProjectileState, const FGSProjectileFrameState& FrameState);

	// Return true if a projectile can be spawned on the target surface
	virtual bool CanSpawnOnTargetHit(const FHitResult& Hit, UGSProjectileState* ProjectileState) const;
	
	// Returns true if the projectile matches the filter so we do not apply this effect to all projectiles
	virtual bool DoesProjectileMatchFilter(UGSProjectileState* Projectile) const;
	
private:
	int32 ActivatedFrame = INDEX_NONE;
	
#if WITH_EDITOR
	FDelegateHandle BlueprintCompiledHandle;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
