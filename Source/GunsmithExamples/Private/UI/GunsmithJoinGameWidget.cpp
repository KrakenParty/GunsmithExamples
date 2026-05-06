// Copyright Kraken Party Limited. All Rights Reserved.


#include "UI/GunsmithJoinGameWidget.h"

#include "GameDelegates.h"
#include "Components/EditableText.h"
#include "Online.h"
#include "OnlineSessionSettings.h"
#include "TimerManager.h"
#include "Components/TextBlock.h"
#include "Engine/Engine.h"
#include "Game/Modes/Multiplayer/GunsmithMultiplayerGameMode.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Online/OnlineSessionNames.h"

void UGunsmithJoinGameWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ErrorTextWidget->SetVisibility(ESlateVisibility::Collapsed);
	
	if (IOnlineSubsystem::DoesInstanceExist(STEAM_SUBSYSTEM))
	{
		DestinationTextWidget->SetHintText(FText::FromString("SESSION ID"));
	}
	else
	{
		DestinationTextWidget->SetHintText(FText::FromString("IP ADDRESS"));
		DestinationTextWidget->SetText(FText::FromString("127.0.0.1"));
	}
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
	
	if (IOnlineSubsystem::DoesInstanceExist(STEAM_SUBSYSTEM))
	{
		if (!IsValidSessionCode(ConnectionString))
		{
			SetInfoString("Please enter a valid session code", true);
			return;
		}
		
		// Disabled session based joining due to not being able to use Steam Sockets
		IOnlineSessionPtr SessionInterface = Online::GetSessionInterface();
		if (SessionInterface)
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
	else
	{
		SetInfoString("Connecting...", false);
		UGameplayStatics::OpenLevel(this, FName(ConnectionString));

		GetWorld()->GetTimerManager().SetTimer(ConnectionTimeoutHandle, FTimerDelegate::CreateUObject(this, &UGunsmithJoinGameWidget::OnConnectionTimeout), 20.0f, false);	
	}
	
	if (!TravelFailedHandle.IsValid())
	{
		TravelFailedHandle = GEngine->TravelFailureEvent.AddUObject(this, &UGunsmithJoinGameWidget::OnTravelFailed);
		FGameDelegates::Get().GetPendingConnectionLostDelegate().AddUObject(this, &UGunsmithJoinGameWidget::OnConnectionLost);
	}
}

void UGunsmithJoinGameWidget::SetInfoString(const FString& NewText, bool bIsError) const
{
	if (ErrorTextWidget)
	{
		ErrorTextWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		ErrorTextWidget->SetColorAndOpacity(bIsError ? FColor::Red : FColor::White);
		ErrorTextWidget->SetText(FText::FromString(NewText));
	}
}

void UGunsmithJoinGameWidget::OnSearchFinished(bool bSuccess)
{
	bIsSearchActive = false;
	
	if (bSuccess && SearchSettings)
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

bool UGunsmithJoinGameWidget::IsValidSessionCode(const FString& SessionString) const
{
	return SessionString.Len() == 5;
}

void UGunsmithJoinGameWidget::OnTravelFailed(UWorld* World, ETravelFailure::Type FailureType,
                                             const FString& FailureString)
{
	GEngine->TravelFailureEvent.Remove(TravelFailedHandle);
	TravelFailedHandle.Reset();
}

void UGunsmithJoinGameWidget::OnConnectionLost(const FUniqueNetIdRepl& ConnectionUniqueId)
{
	OnConnectionTimeout();
}

void UGunsmithJoinGameWidget::OnConnectionTimeout()
{
	SetInfoString("Unable to connect to IP. Please ensure you're connecting to a local IP or have opened the port 7777.", true);
}
