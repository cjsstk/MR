#pragma once

#include "CoreMinimal.h"
#include "MRGameplayAbility.h"
#include "MRAbility_LockOn.generated.h"

class UMRAbilityTask_LockOnTick;

/**
 * 근접무기 록온 토글 어빌리티.
 *
 * 흐름:
 *   Tab 입력 → ActivateAbility → 최적 몬스터 탐색 → Character.State.LockOn 태그 부착
 *           → LockOnTask 시작(매 틱 카메라 보간)
 *   Tab 재입력 → CancelAbility → EndAbility → 태그 제거, 회전 설정 복원
 *   대상 사망/이탈 → OnTargetLost → EndAbility
 *
 * 차단 조건: Dead, Aiming (활 조준 중엔 록온 불가)
 * 취소 어빌리티: Ability.Sprint
 * 활(Bow) 무기 타입이면 ActivateAbility에서 즉시 종료
 */
UCLASS(Blueprintable)
class MR_API UMRAbility_LockOn : public UMRGameplayAbility
{
	GENERATED_BODY()

public:
	UMRAbility_LockOn();

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

	/** 플레이어로부터 탐색할 최대 거리 */
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	float MaxLockOnDistance = 2000.f;

	/** 카메라 정면 기준으로 탐색할 최대 각도(도) */
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	float MaxTargetAngle = 45.f;

	/** 카메라 회전 보간 속도 */
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	float CameraInterpSpeed = 10.f;

private:
	/** 카메라 정면 방향과 거리 기반 가중 스코어로 최적 몬스터 탐색 */
	AActor* FindBestTarget() const;

	UFUNCTION()
	void OnTargetLost();

	UPROPERTY()
	TObjectPtr<UMRAbilityTask_LockOnTick> LockOnTask;
};
