// Copyright Kraken Party Limited. All Rights Reserved.

#pragma once

#include <Interfaces/OnlineSessionInterface.h>

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GunsmithGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class GUNSMITHEXAMPLES_API UGunsmithGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// UGameInstance Begin
	virtual void Init() override;
	virtual void StartRecordingReplay(const FString& InName, const FString& FriendlyName, const TArray<FString>& AdditionalOptions = TArray<FString>(), TSharedPtr<IAnalyticsProvider> AnalyticsProvider = nullptr);
	virtual bool PlayReplay(const FString& InName, UWorld* WorldOverride = nullptr, const TArray<FString>& AdditionalOptions = TArray<FString>()) override;
	// UGameInstance End

private:
	void OnSessionInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);
	void OnJoinFinished(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
};
