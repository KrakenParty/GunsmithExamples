// Copyright Kraken Party Limited. All Rights Reserved.

#include "GunsmithGameInstance.h"

#include "Online.h"


void UGunsmithGameInstance::Init()
{
	Super::Init();

	if (IOnlineSessionPtr SessionInterface = Online::GetSessionInterface())
	{
		SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UGunsmithGameInstance::OnJoinFinished));
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
