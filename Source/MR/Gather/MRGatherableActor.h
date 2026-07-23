// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/MRGatherable.h"
#include "MRGatherableActor.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UAnimMontage;
struct FGatherableTableRow;

/**
 * 광석·식물 등 월드에 배치되는 채집 오브젝트.
 * GatherableTableRow(FGatherableTableRow)를 GatherableType으로 조회해 동작이 결정된다.
 * 몬스터 사체와 달리 채집 소진 후 소멸하지 않고 RespawnDelay 후 재생성된다.
 */
UCLASS()
class MR_API AMRGatherableActor : public AActor, public IMRGatherable
{
	GENERATED_BODY()

public:
	AMRGatherableActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// ─── IMRGatherable ───────────────────────────────────────────────────────

	virtual bool CanBeGathered() const override { return !bDepleted && RemainingGathers > 0; }
	virtual void GetGatherSpec(FMRGatherSpec& OutSpec) const override;
	virtual void PerformGather(UMRAbility_Gather* Ability) override;

protected:
	virtual void BeginPlay() override;

	/** FGatherableTableRow::Type과 매칭되는 채집물 타입 번호 */
	UPROPERTY(EditAnywhere, Category = "Gather")
	int32 GatherableType = 0;

private:
	/** 채집 대상 메시 (루트) */
	UPROPERTY(VisibleAnywhere, Category = "Gather")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	/** 인터랙션 볼륨. 플레이어 접근을 감지해 프롬프트를 띄운다. */
	UPROPERTY(VisibleAnywhere, Category = "Gather")
	TObjectPtr<USphereComponent> InteractionVolume;

	/** BeginPlay에서 로드해 캐시한 채집 몽타주 (GatherSpec의 MontageOverride로 사용) */
	UPROPERTY()
	TObjectPtr<UAnimMontage> CachedMontage;

	/** 캐시된 테이블 Row. CMS의 LoadedTables는 게임 수명 동안 유지되므로 포인터 캐시가 안전하다. */
	const FGatherableTableRow* CachedRow = nullptr;

	/** 남은 채집 횟수 */
	int32 RemainingGathers = 0;

	/** 소진되어 숨김/리스폰 대기 중인지 */
	bool bDepleted = false;

	FTimerHandle RespawnTimerHandle;

	UFUNCTION()
	void OnGatherVolumeOverlapBegin(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnGatherVolumeOverlapEnd(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	/** 채집 소진 처리: 숨김 + 충돌 해제 + 범위 내 플레이어 프롬프트 정리 + 리스폰 타이머 예약 */
	void EnterDepletedAndScheduleRespawn();

	/** 리스폰: 상태/횟수 복원 + 표시 + 충돌 재활성 + 이미 범위 내 플레이어 재감지 */
	void Respawn();
};
