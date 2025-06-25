// Copyright Kraken Party Limited. All Rights Reserved.

#include "UI/GunsmithMultiplayerHUDWidget.h"

#include "Game/Modes/Multiplayer/GunsmithMultiplayerGameState.h"
#include "GameFramework/GameMode.h"
#include "UI/GunsmithActivatableWidget.h"
#include "UI/GunsmithTextDisplayWidget.h"
#include "Weapon/Attachment/GSWeaponAttachment.h"

void UGunsmithMultiplayerHUDWidget::SetEquipmentText(const FGSEquipData& NewEquipData) const
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

bool UGunsmithMultiplayerHUDWidget::Initialize()
{
	bool bIsInitialized = Super::Initialize();

	LobbyOwnerWidget->Hide();
	
	return bIsInitialized;
}

void UGunsmithMultiplayerHUDWidget::UpdateLobbyOwnerWidgetVisibility(bool bShow) const
{
	if (bShow)
	{
		LobbyOwnerWidget->Activate();
	}
	else if (LobbyOwnerWidget->GetIsActive())
	{
		LobbyOwnerWidget->Deactivate();
	}
}
