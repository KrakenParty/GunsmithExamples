// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/GunsmithPlayerController.h"
#include "Weapon/GSEquipData.h"
#include "GunsmithTestPlayerController.generated.h"

/**
 *	A player controller used to start Gunsmith test scenarios
 */
UCLASS()
class GUNSMITHEXAMPLES_API AGunsmithTestPlayerController : public AGunsmithPlayerController
{
	GENERATED_BODY()
	
public:
	// AGunsmithPlayerController Begin
	virtual void SetupInputComponent() override;
	// AGunsmithPlayerController End
	
	// Called to change the servers current auto shoot index for accuracy testing
	UFUNCTION(Server, Reliable, Category="Tests")
	void Server_SetAutoShootIndex(int32 Index, FName BoneName);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> TestControllerMappingContext = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> AddAttachmentInputAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> RemoveAttachmentInputAction = nullptr;

	UPROPERTY(EditAnywhere, Category="Attachments")
	FGSEquipAttachmentData TestAttachmentData;

private:
	void OnAddAttachmentPressed(const FInputActionValue& Value);
	void OnRemoveAttachmentPressed(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
	void Server_AddAttachment(int32 Frame);

	UFUNCTION(Server, Reliable)
	void Server_RemoveAttachment(int32 Frame);
};
