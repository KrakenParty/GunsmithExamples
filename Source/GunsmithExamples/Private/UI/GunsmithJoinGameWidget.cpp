// Copyright Kraken Party Limited. All Rights Reserved.


#include "UI/GunsmithJoinGameWidget.h"

#include "Components/EditableText.h"
#include "Online.h"
#include "OnlineSessionSettings.h"
#include "Components/TextBlock.h"
#include "Game/Modes/Multiplayer/GunsmithMultiplayerGameMode.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Online/OnlineSessionNames.h"

void UGunsmithJoinGameWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ErrorTextWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void UGunsmithJoinGameWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (SearchHandle.IsValid())
	{
		if (IOnlineSessionPtr SessionInterface = Online::GetSessionInterface())
		{
			SessionInterface->OnFindSessionsCompleteDelegates.Remove(SearchHandle);
		}
	}
}

void UGunsmithJoinGameWidget::TravelToDestination()
{
	if (bIsSearchActive)
	{
		return;
	}

	ErrorTextWidget->SetVisibility(ESlateVisibility::Collapsed);
	
	FString ConnectionString = DestinationTextWidget->GetText().ToString();
	ConnectionString = ConnectionString.TrimStartAndEnd();

	IOnlineIdentityPtr IdentityInterface = Online::GetIdentityInterface();
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface();
	if (IdentityInterface && SessionInterface)
	{
		SearchSettings = MakeShared<FOnlineSessionSearch>();
		SearchSettings->QuerySettings.Set(AGunsmithMultiplayerGameMode::SearchParam, ConnectionString, EOnlineComparisonOp::Equals);
		SearchSettings->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

		if (!SearchHandle.IsValid())
		{
			SearchHandle = SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UGunsmithJoinGameWidget::OnSearchFinished);
		}
		
		if (SessionInterface->FindSessions(0, SearchSettings.ToSharedRef()))
		{
			bIsSearchActive = true;
			SetInfoString("Searching...", false);
		}
		else
		{
			SetInfoString("Unable to find session", true);
		}
	}
}

void UGunsmithJoinGameWidget::SetInfoString(const FString& NewText, bool bIsError) const
{
	ErrorTextWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	ErrorTextWidget->SetColorAndOpacity(bIsError ? FColor::Red : FColor::White);
	ErrorTextWidget->SetText(FText::FromString(NewText));
}

void UGunsmithJoinGameWidget::OnSearchFinished(bool bSuccess)
{
	bIsSearchActive = false;
	
	if (bSuccess)
	{
		bool bHasValidSession = false;
		if (IOnlineSessionPtr SessionInterface = Online::GetSessionInterface())
		{
			for (const FOnlineSessionSearchResult& Result : SearchSettings->SearchResults)
			{
				bHasValidSession = true;
				if (SessionInterface->JoinSession(0, AGunsmithMultiplayerGameMode::SessionName, Result))
				{
					SetInfoString("Joining session", false);
				}
				else
				{
					SetInfoString("Unable to join session", true);
				}
				break;
			}
		}

		if (!bHasValidSession)
		{
			SetInfoString("Unable to find matching session", true);
		}
	}
	else
	{
		SetInfoString("Session search failed", true);
	}
}
