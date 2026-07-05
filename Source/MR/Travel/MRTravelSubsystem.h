// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MRTravelTypes.h"
#include "MRStrongId.h"
#include "MRTravelSubsystem.generated.h"

class AMRPlayerCharacter;

/**
 * 마을↔필드 레벨 이동을 관리하는 GameInstanceSubsystem.
 * AMRTravelGate가 RequestTravel / RequestReturnToVillage를 호출하면
 * 플레이어 상태 보존 → 화면 페이드 아웃 → OpenLevel 순서로 실행된다.
 * 새 레벨에서는 GameMode::HandleStartingNewPlayer가 RestorePlayerState를 호출해
 * 페이드 인 + 상태 복원을 처리한다.
 * VillageMapName, FadeDuration 설정은 UMRTravelSettings(Project Settings > MR > Travel)에서 관리한다.
 */
UCLASS()
class MR_API UMRTravelSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * 필드로 이동 요청.
	 * @param WorldContext  호출 컨텍스트 (TravelGate 액터 등)
	 * @param FieldId       FFieldTableRow의 RowName
	 */
	void RequestTravel(const UObject* WorldContext, FFieldId FieldId);

	/** 마을로 귀환 요청 */
	void RequestReturnToVillage(const UObject* WorldContext);

	/**
	 * 새 레벨의 GameMode에서 호출한다.
	 * 저장된 상태를 플레이어에게 적용하고 화면을 페이드 인한다.
	 */
	void RestorePlayerState(AMRPlayerCharacter* Player);

	/** 현재 이동 중인(또는 마지막으로 이동한) 필드 ID */
	FFieldId GetCurrentFieldId() const { return FFieldId(CurrentFieldId); }

private:
	void SavePlayerState(const UObject* WorldContext);
	void ExecuteTravel(const UObject* WorldContext, const FString& MapName);
	void OnFadeOutComplete();

	UPROPERTY()
	FMRPlayerPersistData PersistData;

	UPROPERTY()
	int32 CurrentFieldId = 0;

	FTimerHandle TravelTimerHandle;
	FString PendingMapName;
};
