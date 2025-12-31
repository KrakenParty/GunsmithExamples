// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GunsmithRangeScoreWidget.generated.h"

class UTextBlock;
class UGSWeaponDataAsset;
class UGSShootingComponent;

/**
 *	A widget to show the current score in the range mode
 */
UCLASS()
class GUNSMITHEXAMPLES_API UGunsmithRangeScoreWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// UUserWidget Begin
	virtual void NativeOnInitialized() override;
	// UUserWidget End
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Score Widget", meta=(BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(EditDefaultsOnly, Category="Score Widget", meta=(BindWidget))
	TObjectPtr<UTextBlock> ScoreText;

	UPROPERTY(EditDefaultsOnly, Category="Score Widget", meta=(BindWidget))
	TObjectPtr<UTextBlock> TimeRemainingText;

	UPROPERTY(EditDefaultsOnly, Category="Score Widget", meta=(BindWidget))
	TObjectPtr<UTextBlock> RecordText;

	void OnWeaponChanged(UGSShootingComponent* ShootingComponent, UGSWeaponDataAsset* NewWeaponData);
	void OnScoreChanged(UGSWeaponDataAsset* WeaponData, float Score) const;
	void OnRecordChanged(UGSWeaponDataAsset* WeaponData, float Score) const;
	void OnActiveStateChanged(bool bActive) const;
	void OnTimeRemainingChanged(int32 TimeRemaining, bool bIsWarmUpActive) const;
	void SetupWorld(AGameStateBase* GameState);
};
