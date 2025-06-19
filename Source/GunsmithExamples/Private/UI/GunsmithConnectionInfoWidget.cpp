// Copyright Kraken Party Limited. All Rights Reserved.


#include "UI/GunsmithConnectionInfoWidget.h"

#include "Engine/NetConnection.h"
#include "Online.h"
#include "OnlineSessionSettings.h"
#include "Components/EditableText.h"
#include "Game/GunsmithMultiplayerGameMode.h"
#include "Interfaces/OnlineSessionInterface.h"

void UGunsmithConnectionInfoWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	// No need to show as a client
	if (!GetWorld()->IsNetMode(NM_ListenServer))
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	if (IOnlineSessionPtr SessionInterface = Online::GetSessionInterface())
	{
		SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UGunsmithConnectionInfoWidget::OnSessionCreateComplete);

		if (SessionInterface->GetNamedSession(AGunsmithMultiplayerGameMode::SessionName))
		{
			OnSessionCreateComplete(AGunsmithMultiplayerGameMode::SessionName, true);
		}
	}
}

void UGunsmithConnectionInfoWidget::OnSessionCreateComplete(FName SessionName, bool bIsSuccessful)
{
	SetVisibility(ESlateVisibility::Visible);

	bool bHasSearchParam = false;
	if (bIsSuccessful)
	{
		if (IOnlineSessionPtr SessionInterface = Online::GetSessionInterface())
		{
			if (FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName))
			{
				FString SessionID;

				if (Session->SessionSettings.Get(AGunsmithMultiplayerGameMode::SearchParam, SessionID))
				{
					bHasSearchParam = true;
					TextWidget->SetText(FText::FromString(SessionID));
				}
			}
		}
	}
	
	if (!bHasSearchParam)
	{
		TextWidget->SetText(FText::FromString("Unable to host"));
	}
}
