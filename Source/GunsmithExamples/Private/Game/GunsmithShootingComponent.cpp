// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/GunsmithShootingComponent.h"

#include "GSDeveloperSettings.h"
#include "Engine/World.h"
#include "Game/GunsmithPlayerController.h"
#include "Game/Effects/NumberPops/LyraNumberPopComponent_NiagaraText.h"
#include "GameFramework/HUD.h"
#include "Health/GSHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Netcode/GSNetworkLibrary.h"
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

void UGunsmithShootingComponent::NotifyDamageDealt_Implementation(const FGSDamageRecord& DamageRecord, bool bIsPredicted)
{
	Super::NotifyDamageDealt_Implementation(DamageRecord, bIsPredicted);
	
	// Show the damage number on the target
	if (IsLocallyControlled() && (bIsPredicted == UGSDeveloperSettings::Get()->bPredictDamageApplication || UGSNetworkLibrary::HasAuthority(this)))
	{
		APlayerController* LocalPlayerController = UGameplayStatics::GetPlayerController(this, 0);
		if (AGunsmithPlayerController* GunsmithPlayerController = Cast<AGunsmithPlayerController>(LocalPlayerController))
		{
			if (ULyraNumberPopComponent_NiagaraText* NumberPopComponent = GunsmithPlayerController->GetNumberPopComponent())
			{			
				FLyraNumberPopRequest Request;
				Request.WorldLocation = DamageRecord.HitLocation;
				Request.NumberToDisplay = FMath::RoundToInt(DamageRecord.Damage);
				Request.bIsCriticalDamage = DamageRecord.DamageMultiplier > 1.5f;
			
				NumberPopComponent->AddNumberPop(Request);
			}
		}
	}
}

void UGunsmithShootingComponent::OnGlobalShowUI(bool bShow)
{
	if (CrosshairWidget)
	{
		CrosshairWidget->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	
	ShowUIDelegate.Broadcast(bShow);
}
