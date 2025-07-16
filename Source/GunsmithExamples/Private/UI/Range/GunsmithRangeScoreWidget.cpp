// Copyright Kraken Party Limited. All Rights Reserved.

#include "UI/Range/GunsmithRangeScoreWidget.h"

#include "GSGameplayLibrary.h"
#include "Components/TextBlock.h"
#include "Game/Modes/Range/GunsmithGameState_Range.h"
#include "Weapon/GSShootingComponent.h"

void UGunsmithRangeScoreWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (AGunsmithGameState_Range* GameState = GetWorld()->GetGameState<AGunsmithGameState_Range>())
	{
		GameState->OnScoreChanged.AddUObject(this, &UGunsmithRangeScoreWidget::OnScoreChanged);
		GameState->OnRecordChanged.AddUObject(this, &UGunsmithRangeScoreWidget::OnRecordChanged);
		GameState->OnWeaponChanged.AddUObject(this, &UGunsmithRangeScoreWidget::OnWeaponChanged);
		GameState->OnPractiseActiveChanged.AddUObject(this, &UGunsmithRangeScoreWidget::OnActiveStateChanged);
		GameState->OnPractiseTimeRemainingChanged.AddUObject(this, &UGunsmithRangeScoreWidget::OnTimeRemainingChanged);

		OnWeaponChanged(nullptr, GameState->GetTrackedWeapon());
		OnActiveStateChanged(GameState->IsPractiseActive());
	}
}

void UGunsmithRangeScoreWidget::OnWeaponChanged(UGSShootingComponent* ShootingComponent, UGSWeaponDataAsset* NewWeaponData)
{
	if (!NewWeaponData)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	SetVisibility(ESlateVisibility::HitTestInvisible);

	TitleText->SetText(NewWeaponData->Name);

	if (AGunsmithGameState_Range* GameState = GetWorld()->GetGameState<AGunsmithGameState_Range>())
	{
		OnRecordChanged(NewWeaponData, GameState->GetRecordForWeapon(NewWeaponData));
	}
}

void UGunsmithRangeScoreWidget::OnScoreChanged(UGSWeaponDataAsset* WeaponData, float Score) const
{
	ScoreText->SetText(FText::FromString(FString::FromInt(Score)));
}

void UGunsmithRangeScoreWidget::OnRecordChanged(UGSWeaponDataAsset* WeaponData, float Score) const
{
	RecordText->SetText(FText::FromString(Score > 0.0f ? FString::FromInt(Score) : "N/A"));
}

void UGunsmithRangeScoreWidget::OnActiveStateChanged(bool bActive) const
{
	if (bActive)
	{
		OnTimeRemainingChanged(30.0f, true);
	}
	else
	{
		TimeRemainingText->SetText(FText::FromString("N/A"));
	}
}

void UGunsmithRangeScoreWidget::OnTimeRemainingChanged(int32 TimeRemaining, bool bIsWarmUpActive) const
{
	if (bIsWarmUpActive)
	{
		TimeRemainingText->SetText(FText::FromString("Starting..."));
	}
	else
	{
		TimeRemainingText->SetText(FText::FromString(FString::FromInt(TimeRemaining)));
	}
}
