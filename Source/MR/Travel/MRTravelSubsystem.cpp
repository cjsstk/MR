// Fill out your copyright notice in the Description page of Project Settings.

#include "MRTravelSubsystem.h"
#include "Character/MRPlayerCharacter.h"
#include "Subsystem/CMSSubsystem.h"
#include "Settings/MRTravelSettings.h"
#include "MRDataTable.h"
#include "Sugar.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void UMRTravelSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UMRTravelSubsystem::RequestTravel(const UObject* WorldContext, FName FieldId)
{
	if (!WorldContext)
	{
		return;
	}

	UCMSSubsystem* CMS = GetCMS(WorldContext);
	if (!CMS)
	{
		UE_LOG(LogTemp, Error, TEXT("UMRTravelSubsystem::RequestTravel - CMS를 찾을 수 없음"));
		return;
	}

	FFieldTableRow* FieldRow = CMS->GetFieldRow(FieldId);
	if (!FieldRow)
	{
		UE_LOG(LogTemp, Error, TEXT("UMRTravelSubsystem::RequestTravel - FieldRow '%s' 없음"), *FieldId.ToString());
		return;
	}

	CurrentFieldId = FieldId;
	SavePlayerState(WorldContext);
	ExecuteTravel(WorldContext, FieldRow->MapName);
}

void UMRTravelSubsystem::RequestReturnToVillage(const UObject* WorldContext)
{
	if (!WorldContext)
	{
		return;
	}

	CurrentFieldId = NAME_None;
	SavePlayerState(WorldContext);
	ExecuteTravel(WorldContext, UMRTravelSettings::Get()->VillageMapName);
}

void UMRTravelSubsystem::RestorePlayerState(AMRPlayerCharacter* Player)
{
	if (Player && PersistData.bIsValid)
	{
		Player->ApplyPersistData(PersistData);
		PersistData.bIsValid = false;
	}

	// 페이드 인 — Player가 nullptr이어도 항상 실행해 화면이 검게 고정되지 않도록 한다
	UWorld* World = Player ? Player->GetWorld() : GetGameInstance()->GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (PC && PC->PlayerCameraManager)
	{
		PC->PlayerCameraManager->StartCameraFade(1.f, 0.f, UMRTravelSettings::Get()->FadeDuration, FLinearColor::Black, false, false);
	}
}

void UMRTravelSubsystem::SavePlayerState(const UObject* WorldContext)
{
	UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	AMRPlayerCharacter* Player = Cast<AMRPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
	if (!Player)
	{
		return;
	}

	PersistData = Player->ExtractPersistData();
	PersistData.bIsValid = true;
}

void UMRTravelSubsystem::ExecuteTravel(const UObject* WorldContext, const FString& MapName)
{
	UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	// 중복 호출 방지
	if (World->GetTimerManager().IsTimerActive(TravelTimerHandle))
	{
		return;
	}

	// 페이드 아웃
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (PC && PC->PlayerCameraManager)
	{
		PC->PlayerCameraManager->StartCameraFade(0.f, 1.f, UMRTravelSettings::Get()->FadeDuration, FLinearColor::Black, false, true);
	}

	PendingMapName = MapName;
	World->GetTimerManager().SetTimer(
		TravelTimerHandle,
		this, &UMRTravelSubsystem::OnFadeOutComplete,
		UMRTravelSettings::Get()->FadeDuration,
		false
	);
}

void UMRTravelSubsystem::OnFadeOutComplete()
{
	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	UGameplayStatics::OpenLevel(World, FName(*PendingMapName));
}
