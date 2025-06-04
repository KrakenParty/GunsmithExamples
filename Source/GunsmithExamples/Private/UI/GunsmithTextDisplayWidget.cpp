// Copyright Kraken Party Limited. All Rights Reserved.

#include "UI/GunsmithTextDisplayWidget.h"

#include "Components/TextBlock.h"

void UGunsmithTextDisplayWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	TitleTextBlock->SetOpacity(0.0f);
	DescriptionTextBlock->SetOpacity(0.0f);
}

void UGunsmithTextDisplayWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	const float AlphaDelta = (bIsFadingIn ? 1.0f * FadeInSpeed : -1.0f * FadeOutSpeed) * InDeltaTime;
	const float NewAlpha = FMath::Clamp(CurrentTextAlpha + AlphaDelta, 0.0f, 1.0f);

	if (NewAlpha != CurrentTextAlpha)
	{
		TitleTextBlock->SetOpacity(NewAlpha);
		DescriptionTextBlock->SetOpacity(NewAlpha);
	}
	
	if (bIsFadingIn && FMath::IsNearlyEqual(NewAlpha, 1.0f))
	{
		TimeFadedIn += InDeltaTime;

		if (TimeFadedIn > TimeToDisplayText)
		{
			bIsFadingIn = false;
		}
	}

	CurrentTextAlpha = NewAlpha;
}

void UGunsmithTextDisplayWidget::DisplayText(const FString& NewTitle, const FString& NewText)
{
	TitleTextBlock->SetText(FText::FromString(NewTitle));
	DescriptionTextBlock->SetText(FText::FromString(NewText));

	bIsFadingIn = true;
	TimeFadedIn = 0.0f;
}
