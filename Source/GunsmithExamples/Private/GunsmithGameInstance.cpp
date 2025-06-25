// Copyright Kraken Party Limited. All Rights Reserved.

#include "GunsmithGameInstance.h"

#include "Online.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "Game/Modes/Multiplayer/GunsmithMultiplayerGameMode.h"


void UGunsmithGameInstance::Init()
{
	Super::Init();

	if (IOnlineSessionPtr SessionInterface = Online::GetSessionInterface())
	{
		SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &UGunsmithGameInstance::OnSessionInviteAccepted));
		SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UGunsmithGameInstance::OnJoinFinished));
	}
}

void UGunsmithGameInstance::OnSessionInviteAccepted(const bool bWasSuccessful, const int32 ControllerId,
	FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult)
{
	if (bWasSuccessful)
	{
		IOnlineSessionPtr SessionInterface = Online::GetSessionInterface();
		if (SessionInterface)
		{	
			SessionInterface->JoinSession(0, AGunsmithMultiplayerGameMode::SessionName, InviteResult);
		}
	}
}

void UGunsmithGameInstance::OnJoinFinished(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		if (IOnlineSessionPtr SessionInterface = Online::GetSessionInterface())
		{
			ClientTravelToSession(0, SessionName);
		}
	}
}
