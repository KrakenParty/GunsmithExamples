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
	
	UWorld* World = GetWorld();
	if (AGunsmithGameState_Range* GameState = World->GetGameState<AGunsmithGameState_Range>())
	{
		SetupWorld(GameState);
	}
	else
	{
		World->GameStateSetEvent.AddUObject(this, &UGunsmithRangeStartWidget::SetupWorld);	
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

void UGunsmithRangeStartWidget::SetupWorld(AGameStateBase* GameState)
{
	if (AGunsmithGameState_Range* RangeGameState = Cast<AGunsmithGameState_Range>(GameState))
	{
		RangeGameState->OnPractiseActiveChanged.AddUObject(this, &UGunsmithRangeStartWidget::OnPractiseActivityChanged);

		OnPractiseActivityChanged(RangeGameState->IsPractiseActive());
	}
}
