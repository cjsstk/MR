// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MREnum.h"
#include "MRGatherable.generated.h"

class AActor;
class UAnimMontage;
class UMRAbility_Gather;

/**
 * 채집 대상의 행동 차이를 데이터로 표현하는 스펙.
 * 순수 struct(비-USTRUCT) — 어빌리티/오버랩 로직에서 값으로 주고받는다.
 */
struct FMRGatherSpec
{
	/** 채집 중 이동 처리 방식 (정지+전신 vs 이동+상체) */
	EMRGatherMovementPolicy MovementPolicy = EMRGatherMovementPolicy::Stationary;

	/** 채집 대상 종류 */
	EMRGatherType GatherType = EMRGatherType::Monster;

	/** 이 대상 전용 몽타주. 지정되면 어빌리티 기본 몽타주 대신 사용한다. */
	TObjectPtr<UAnimMontage> MontageOverride = nullptr;

	/** 인터랙션 프롬프트에 표시할 텍스트 (e.g. "박리하기", "채광") */
	FText InteractionText;
};

UINTERFACE(MinimalAPI)
class UMRGatherable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 채집 가능한 대상이 구현하는 인터페이스.
 * 몬스터 사체·광석·식물 등이 이 인터페이스를 구현하면 공용 채집 어빌리티로 처리된다.
 */
class IMRGatherable
{
	GENERATED_BODY()

public:
	/** 현재 채집 가능한 상태인지 */
	virtual bool CanBeGathered() const = 0;

	/** 채집 동작 파라미터(이동정책·몽타주·텍스트 등)를 채워 반환한다. */
	virtual void GetGatherSpec(FMRGatherSpec& OutSpec) const = 0;

	/** 실제 채집 수행 — 드롭 계산 + 인벤토리 지급 + 사후처리(소멸/리스폰 카운트). */
	virtual void PerformGather(UMRAbility_Gather* Ability) = 0;
};

/**
 * 오버랩 → 프롬프트 표시/숨김 공용 헬퍼.
 * IMRGatherable을 구현한 액터의 인터랙션 볼륨 오버랩 콜백에서 그대로 호출한다.
 */
namespace MRGatherableInteract
{
	/** 플레이어가 채집 대상 범위에 진입했을 때 프롬프트 표시 + 대상 등록. */
	MR_API void EnterRange(AActor* Gatherable, AActor* OtherActor);

	/** 플레이어가 채집 대상 범위에서 벗어났을 때 프롬프트 숨김 + 대상 해제. */
	MR_API void ExitRange(AActor* Gatherable, AActor* OtherActor);
}
