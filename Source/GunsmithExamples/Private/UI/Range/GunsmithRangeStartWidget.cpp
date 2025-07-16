// Copyright Kraken Party Limited. All Rights Reserved.

#include "UI/Range/GunsmithRangeStartWidget.h"

#include "Components/RichTextBlock.h"
#include "Game/GunsmithPlayerController.h"
#include "Game/Modes/Range/GunsmithGameState_Range.h"

void UGunsmithRangeStartWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (AGunsmithPlayerController* Controller = GetOwningPlayer<AGunsmithPlayerController>())
	{
		Controller->OnDeviceChanged.AddDynamic(this, &UGunsmithRangeStartWidget::OnDeviceChanged);
	}
	
	if (AGunsmithGameState_Range* GameState = GetWorld()->GetGameState<AGunsmithGameState_Range>())
	{
		GameState->OnPractiseActiveChanged.AddUObject(this, &UGunsmithRangeStartWidget::OnPractiseActivityChanged);

		OnPractiseActivityChanged(GameState->IsPractiseActive());
	}
}

void UGunsmithRangeStartWidget::OnPractiseActivityChanged(bool bIsActive) const
{
	bool bIsUsingGamepad = false;
	if (AGunsmithPlayerController* Controller = GetOwningPlayer<AGunsmithPlayerController>())
	{
		bIsUsingGamepad = Controller->WasLastUsingGamepad();
	}
	const FText& Text = bIsUsingGamepad ? (bIsActive ? ActiveTextGamepad : InactiveTextGamepad) : (bIsActive ? ActiveTextPC : InactiveTextPC);
	TextWidget->SetText(Text);
}

void UGunsmithRangeStartWidget::OnDeviceChanged(bool bIsUsingGamepad)
{
	if (AGunsmithGameState_Range* GameState = GetWorld()->GetGameState<AGunsmithGameState_Range>())
	{
		OnPractiseActivityChanged(GameState->IsPractiseActive());
	}
}
