// Copyright Kraken Party Limited. All Rights Reserved.


#include "UI/GunsmithConnectionInfoWidget.h"

#include "Engine/NetConnection.h"
#include "Online.h"
#include "OnlineSessionSettings.h"
#include "SocketSubsystem.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "Game/Modes/Multiplayer/GunsmithMultiplayerGameMode.h"
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

	if (IOnlineSubsystem::DoesInstanceExist(STEAM_SUBSYSTEM))
	{
		if (IOnlineSessionPtr SessionInterface = Online::GetSessionInterface())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UGunsmithConnectionInfoWidget::OnSessionCreateComplete);

			if (SessionInterface->GetNamedSession(AGunsmithMultiplayerGameMode::SessionName))
			{
				OnSessionCreateComplete(AGunsmithMultiplayerGameMode::SessionName, true);
			}
		}
		else
		{
			TextWidget->SetText(FText::FromString("Unable to host"));
		}
		
		SessionTypeText->SetText(FText::FromString("Session ID:"));
		JoiningInfoText->SetText(FText::FromString("Please ensure any joining players are logged into Steam"));
		
		return;
	}
	
	TArray<TSharedPtr<FInternetAddr>> AllAddresses;
	ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalAdapterAddresses(AllAddresses);
	
	TArray<FString> AddressStrings;
	for (const TSharedPtr<FInternetAddr>& Address : AllAddresses)
	{
		AddressStrings.Emplace(Address->ToString(false));
	}
	
	FString FullString = "";
	if (AddressStrings.Num() > 0)
	{
		for (const FString& Address : AddressStrings)
		{
			FullString += Address + "\n";
		}
	}
	else
	{
		FullString = "Unable to host";
	}

	TextWidget->SetText(FText::FromString(FullString));
	
	SessionTypeText->SetText(FText::FromString("IP Addresses:"));
	JoiningInfoText->SetText(FText::FromString("Please ensure any joining players are NOT logged into Steam"));
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
