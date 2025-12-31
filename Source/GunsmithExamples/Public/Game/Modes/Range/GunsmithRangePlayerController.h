// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/GunsmithPlayerController.h"
#include "GunsmithRangePlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

/**
 *	A Gunsmith Player controller set up with inputs for the range mode
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithRangePlayerController : public AGunsmithPlayerController
{
	GENERATED_BODY()

public:
	// AGunsmithPlayerController Begin
	virtual void SetupInputComponent() override;
	// AGunsmithPlayerController End
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> RangeMappingContext = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> TogglePractiseInputAction = nullptr;

	void OnTogglePractisePressed(const FInputActionValue& Value);
	
	UFUNCTION(Server, Reliable)
	void Server_TogglePractise();
};
