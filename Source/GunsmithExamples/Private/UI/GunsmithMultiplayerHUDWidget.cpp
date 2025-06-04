// Copyright Kraken Party Limited. All Rights Reserved.

#include "UI/GunsmithMultiplayerHUDWidget.h"

#include "Game/GunsmithMultiplayerGameState.h"
#include "Game/GunsmithMultiplayerPC.h"
#include "GameFramework/GameMode.h"
#include "UI/GunsmithTextDisplayWidget.h"
#include "Weapon/Attachment/GSWeaponAttachment.h"

bool UGunsmithMultiplayerHUDWidget::Initialize()
{
	bool bIsInitialized = Super::Initialize();
	
	if (AGunsmithMultiplayerPC* Controller = Cast<AGunsmithMultiplayerPC>(GetOwningPlayer()))
	{
		Controller->OnLobbyOwnershipChanged.AddUObject(this, &UGunsmithMultiplayerHUDWidget::OnLobbyOwnershipChanged);
		OnLobbyOwnershipChanged(Controller->GetIsLobbyOwner());
	}

	UWorld* World = GetWorld();
	if (AGunsmithMultiplayerGameState* GameState =  World->GetGameState<AGunsmithMultiplayerGameState>())
	{
		OnGameStateSet(GameState);
	}
	else
	{
		World->GameStateSetEvent.AddUObject(this, &UGunsmithMultiplayerHUDWidget::OnGameStateSet);
	}
	
	return bIsInitialized;
}

void UGunsmithMultiplayerHUDWidget::OnLobbyOwnershipChanged(bool bIsOwner)
{
	bIsLobbyOwner = bIsOwner;

	UpdateLobbyOwnerWidgetVisibility();
}

void UGunsmithMultiplayerHUDWidget::OnMatchStateChanged(FName NewState)
{
	bIsInPreMatchState = NewState == MatchState::WaitingToStart;
	
	UpdateLobbyOwnerWidgetVisibility();
}

void UGunsmithMultiplayerHUDWidget::OnGameStateSet(AGameStateBase* GameStateBase)
{
	if (AGunsmithMultiplayerGameState* GameState =  Cast<AGunsmithMultiplayerGameState>(GameStateBase))
	{
		GameState->OnStateChangedDelegate.AddUObject(this, &UGunsmithMultiplayerHUDWidget::OnMatchStateChanged);
		GameState->OnEquipmentChangedDelegate.AddUObject(this, &UGunsmithMultiplayerHUDWidget::OnGameStateEquipmentChanged);
		OnMatchStateChanged(GameState->GetMatchState());
	}
}

void UGunsmithMultiplayerHUDWidget::OnGameStateEquipmentChanged(const FGSEquipData& NewEquipData)
{
	if (TextDisplayWidget)
	{
		const FGameplayTag WeaponTag = NewEquipData.WeaponData ? NewEquipData.WeaponData->UniqueTag : FGameplayTag::EmptyTag;

		if (WeaponTag.IsValid())
		{
			FString LeftSplit, RightSplit;
			if (WeaponTag.ToString().Split(".", &LeftSplit, &RightSplit, ESearchCase::CaseSensitive, ESearchDir::FromEnd))
			{
				FString WeaponName = RightSplit;
				FString Title = FString::Format(TEXT("New Weapon: {0}"), { WeaponName });
				FString AttachmentText;
				
				for (const FGSEquipAttachmentData& AttachmentData : NewEquipData.Attachments)
				{
					if (!AttachmentData.Attachment)
					{
						continue;
					}
					
					FString CurrentAttachmentString = FString::Format(TEXT("Attachment: {0}"), { AttachmentData.Attachment->GetDefaultObject<UGSWeaponAttachment>()->AttachmentName.ToString() });
					
					if (AttachmentText.IsEmpty())
					{
						AttachmentText = CurrentAttachmentString;
					}
					else
					{
						// Append to existing string
						AttachmentText = FString::Format(TEXT("{0} \n{1}"), { AttachmentText, CurrentAttachmentString });
					}
				}
		
				TextDisplayWidget->DisplayText(Title, AttachmentText);
			}
		}
	}
}

void UGunsmithMultiplayerHUDWidget::UpdateLobbyOwnerWidgetVisibility() const
{
	const bool bShouldShow = bIsLobbyOwner && bIsInPreMatchState;

	LobbyOwnerWidget->SetVisibility(bShouldShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}
