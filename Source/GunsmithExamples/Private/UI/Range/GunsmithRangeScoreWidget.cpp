// Copyright Kraken Party Limited. All Rights Reserved.

#include "UI/Range/GunsmithRangeScoreWidget.h"

#include "GSGameplayLibrary.h"
#include "Components/TextBlock.h"
#include "Game/Modes/Range/GunsmithGameState_Range.h"
#include "Weapon/GSShootingComponent.h"

void UGunsmithRangeScoreWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UWorld* World = GetWorld();
	if (AGunsmithGameState_Range* GameState = World->GetGameState<AGunsmithGameState_Range>())
	{
		SetupWorld(GameState);
	}
	else
	{
		World->GameStateSetEvent.AddUObject(this, &UGunsmithRangeScoreWidget::SetupWorld);	
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

void UGunsmithRangeScoreWidget::SetupWorld(AGameStateBase* GameState)
{
	if (AGunsmithGameState_Range* RangeGameState = Cast<AGunsmithGameState_Range>(GameState))
	{
		RangeGameState->OnScoreChanged.AddUObject(this, &UGunsmithRangeScoreWidget::OnScoreChanged);
		RangeGameState->OnRecordChanged.AddUObject(this, &UGunsmithRangeScoreWidget::OnRecordChanged);
		RangeGameState->OnWeaponChanged.AddUObject(this, &UGunsmithRangeScoreWidget::OnWeaponChanged);
		RangeGameState->OnPractiseActiveChanged.AddUObject(this, &UGunsmithRangeScoreWidget::OnActiveStateChanged);
		RangeGameState->OnPractiseTimeRemainingChanged.AddUObject(this, &UGunsmithRangeScoreWidget::OnTimeRemainingChanged);

		OnWeaponChanged(nullptr, RangeGameState->GetTrackedWeapon());
		OnActiveStateChanged(RangeGameState->IsPractiseActive());
	}
}
