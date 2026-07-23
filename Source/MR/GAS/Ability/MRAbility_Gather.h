// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRGameplayAbility.h"
#include "Interface/MRGatherable.h"
#include "MRAbility_Gather.generated.h"

class UAbilityTask_PlayMontageAndWait;

/**
 * IMRGatherable 대상(몬스터 사체/광석/식물)에서 자원을 채집하는 공용 어빌리티.
 *
 * 흐름:
 *   ActivateAbility → 대상의 GatherSpec 조회 → Character.State.Gathering 태그 부여
 *   → (Stationary면 이동 입력 차단) → 채집 몽타주 재생
 *   → 몽타주 완료 시 Target->PerformGather(this) 호출
 *   → 드롭 지급 + 결과 팝업 → EndAbility (태그 제거 / 이동 복원)
 *
 * BP 설정 항목:
 *   - StationaryMontage: 정지형(전신) 채집 몽타주 (몬스터 박리 등)
 *   - UpperBodyMontage : 이동형(상체) 채집 몽타주 (광석/식물 등)
 *   대상이 GatherSpec.MontageOverride를 지정하면 그 몽타주가 우선한다.
 */
UCLASS(Blueprintable)
class MR_API UMRAbility_Gather : public UMRGameplayAbility
{
	GENERATED_BODY()

public:
	UMRAbility_Gather();

	/** AMRPlayerCharacter에서 채집 대상을 설정 후 어빌리티 활성화 */
	void SetTargetGatherable(AActor* InTarget);

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	/** 정지형(전신) 채집 몽타주. BP에서 설정. */
	UPROPERTY(EditDefaultsOnly, Category = "Gather")
	TObjectPtr<UAnimMontage> StationaryMontage;

	/** 이동형(상체) 채집 몽타주. BP에서 설정. */
	UPROPERTY(EditDefaultsOnly, Category = "Gather")
	TObjectPtr<UAnimMontage> UpperBodyMontage;

private:
	/** 현재 채집 대상 액터 */
	UPROPERTY()
	TWeakObjectPtr<AActor> TargetActor;

	/** 대상을 IMRGatherable로 변환해 반환. 유효하지 않으면 nullptr. */
	IMRGatherable* GetTarget() const;

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	/** 정지형 채집으로 이동 입력을 막았는지 여부. EndAbility에서 안전하게 복원하기 위한 플래그. */
	bool bDidBlockMovement = false;

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();
};
