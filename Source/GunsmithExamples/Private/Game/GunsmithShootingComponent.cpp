// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/GunsmithShootingComponent.h"

#include "Engine/World.h"
#include "GameFramework/HUD.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/Crosshair/GSCrosshairWidget.h"

FGunsmithShowUIDelegate UGunsmithShootingComponent::GlobalShowUIDelegate;

static FAutoConsoleCommandWithWorldAndArgs FCmdGunsmithShowUI
(
	TEXT("Gunsmith.ShowUI"),
	TEXT("Shows or hides all UI for taking beauty shots"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& InParams, const UWorld* InWorld)
	{
		if(!InWorld)
		{
			return;
		}
		
		const bool bShow = InParams.Num() > 0 && InParams[0].ToBool();

		UGunsmithShootingComponent::GlobalShowUIDelegate.Broadcast(bShow);
		
		if (APlayerController* PlayerController = Cast<APlayerController>(UGameplayStatics::GetPlayerController(InWorld, 0)))
		{
			if (AHUD* HUD = PlayerController->GetHUD())
			{
				if (HUD->bShowHUD != bShow)
				{
					HUD->ShowHUD();
				}
			}
		}
	}),
	ECVF_Cheat
);

void UGunsmithShootingComponent::BeginPlay()
{
	Super::BeginPlay();

	GlobalShowUIDelegate.AddDynamic(this, &UGunsmithShootingComponent::OnGlobalShowUI);
}

void UGunsmithShootingComponent::OnGlobalShowUI(bool bShow)
{
	if (CrosshairWidget)
	{
		CrosshairWidget->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	
	ShowUIDelegate.Broadcast(bShow);
}
