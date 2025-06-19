// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GunsmithActivatableWidget.h"
#include "GunsmithJoinGameWidget.generated.h"

class UTextBlock;
class UEditableText;
class FOnlineSessionSearch;

/**
 *	A popup widget to join another users game
 */
UCLASS()
class GUNSMITHEXAMPLES_API UGunsmithJoinGameWidget : public UGunsmithActivatableWidget
{
	GENERATED_BODY()

public:
	// UUserWidget Begin
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
	// UUserWidget End
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Gunsmith", meta=(BindWidget))
	TObjectPtr<UEditableText> DestinationTextWidget;

	UPROPERTY(EditDefaultsOnly, Category="Gunsmith", meta=(BindWidget))
	TObjectPtr<UTextBlock> ErrorTextWidget;

	// Attempt to travel to the destination
	UFUNCTION(BlueprintCallable, Category="Gunsmith")
	void TravelToDestination();

	void SetInfoString(const FString& NewText, bool bIsError) const;

	void OnSearchFinished(bool bSuccess);

private:
	FDelegateHandle SearchHandle;
	bool bIsSearchActive = false;
	TSharedPtr<FOnlineSessionSearch> SearchSettings = nullptr;
};
