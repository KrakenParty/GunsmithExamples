// Copyright Kraken Party Limited. All Rights Reserved.

#include "GunsmithGameInstance.h"

#include "GSGameState.h"
#include "Online.h"
#include "OnlineSessionSettings.h"
#include "Engine/World.h"
#include "Online/OnlineSessionNames.h"
#include "Game/Modes/Multiplayer/GunsmithMultiplayerGameMode.h"
#include "Kismet/GameplayStatics.h"


void UGunsmithGameInstance::Init()
{
	Super::Init();

	if (IOnlineSessionPtr SessionInterface = Online::GetSessionInterface())
	{
		SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &UGunsmithGameInstance::OnSessionInviteAccepted));
		SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UGunsmithGameInstance::OnJoinFinished));
	}
}

void UGunsmithGameInstance::StartRecordingReplay(const FString& InName, const FString& FriendlyName,
	const TArray<FString>& AdditionalOptions, TSharedPtr<IAnalyticsProvider> AnalyticsProvider)
{
	Super::StartRecordingReplay(InName, FriendlyName, AdditionalOptions, AnalyticsProvider);

	if (AGSGameState* GameState = GetWorld()->GetGameState<AGSGameState>())
	{
		GameState->SaveCurrentFrameForReplays();
	}
}

bool UGunsmithGameInstance::PlayReplay(const FString& InName, UWorld* WorldOverride,
                                       const TArray<FString>& AdditionalOptions)
{
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		PlayerController->ChangeState(NAME_Spectating);
	}
	
	return Super::PlayReplay(InName, WorldOverride, AdditionalOptions);
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
