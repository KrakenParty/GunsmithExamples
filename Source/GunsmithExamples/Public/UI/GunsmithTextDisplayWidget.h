// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GunsmithTextDisplayWidget.generated.h"

class UTextBlock;
/**
 *	A widget used to display game mode text to players
 */
UCLASS()
class GUNSMITHEXAMPLES_API UGunsmithTextDisplayWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// Changes the currently displayed text and begins fading in
	void DisplayText(const FString& NewTitle, const FString& NewText);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TitleTextBlock;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> DescriptionTextBlock;

	// How long to display the text for
	UPROPERTY(EditAnywhere, Category="TextDisplay")
	float TimeToDisplayText = 8.0f;

	// How quickly to fade the text in
	UPROPERTY(EditAnywhere, Category="TextDisplay")
	float FadeInSpeed = 4.0f;

	// How quickly to fade the text out
	UPROPERTY(EditAnywhere, Category="TextDisplay")
	float FadeOutSpeed = 2.0f;

	bool bIsFadingIn = false;
	float CurrentTextAlpha = 0.0f;
	float TimeFadedIn = 0.0f;
};
