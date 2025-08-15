// Copyright Kraken Party Limited. All Rights Reserved.

#include "Game/Modes/Multiplayer/GunsmithMultiplayerGameState.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Netcode/GSNetworkLibrary.h"
#include "Weapon/GSWeaponsSubsystem.h"

void AGunsmithMultiplayerGameState::BeginPlay()
{
	Super::BeginPlay();

	// Send all preload requirements to the weapons subsystem to pool any required objects
	if (UGSWeaponsSubsystem* WeaponsSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UGSWeaponsSubsystem>())
	{
		PreloadDataTable = NewObject<UDataTable>();
		
		FGSPreloadData DynamicPreloadData;
		DynamicPreloadData.PreloadDataTable = PreloadDataTable;
		DynamicPreloadData.PreloadDataTable->RowStruct = FGSPreloadDataRow::StaticStruct();
		DynamicPreloadData.ExpectedPlayers = PreloadData.ExpectedPlayers;

		for (UGSWeaponDataAsset* WeaponData : WeaponPool)
		{
			FGSPreloadDataRow Row;
			Row.WeaponData = WeaponData;
			
			DynamicPreloadData.PreloadDataTable->AddRow(FName(WeaponData->GetName()), Row);
		}

		FGSPreloadDataRow AttachmentsRow;
		for (const TSubclassOf<UGSWeaponAttachment>& Attachment : AttachmentPool)
		{
			FGSPreloadAttachmentData AttachmentData;
			AttachmentData.Attachment = Attachment;
			
			AttachmentsRow.Attachments.Emplace(AttachmentData);
		}

		DynamicPreloadData.PreloadDataTable->AddRow(FName("Attachments"), AttachmentsRow);
		
		WeaponsSubsystem->PreloadObjects(DynamicPreloadData);
	}
}

void AGunsmithMultiplayerGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, CurrentRoundEquipment, Params);
}

void AGunsmithMultiplayerGameState::SetRoundEquipment(const FGSEquipData& NewEquipData)
{
	if (!UGSNetworkLibrary::HasAuthority(this))
	{
		return;
	}
	
	CurrentRoundEquipment = NewEquipData;

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, CurrentRoundEquipment, this);

	OnRep_CurrentRoundEquipment();
}

void AGunsmithMultiplayerGameState::OnRep_MatchState()
{
	Super::OnRep_MatchState();

	OnStateChangedDelegate.Broadcast(MatchState);
}

void AGunsmithMultiplayerGameState::OnRep_CurrentRoundEquipment() const
{
	OnEquipmentChangedDelegate.Broadcast(CurrentRoundEquipment);	
}
