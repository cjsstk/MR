// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRGameplayAbility.h"
#include "MRAbility_Carve.generated.h"

class AMRMonster;
class UAbilityTask_PlayMontageAndWait;

/**
 * 사망한 몬스터 사체에서 소재를 박리하는 어빌리티.
 *
 * 흐름:
 *   ActivateAbility → Character.State.Carving 태그 부여
 *   → PlayMontageAndWait (카빙 몽타주)
 *   → 몽타주 완료 시 TargetMonster->PerformCarve(this) 호출
 *   → 인벤토리에 소재 지급 → 결과 팝업 표시
 *   → EndAbility (Carving 태그 제거)
 *
 * BP 설정 항목:
 *   - CarveMontage: 박리 애니메이션 몽타주
 */
UCLASS(Blueprintable)
class MR_API UMRAbility_Carve : public UMRGameplayAbility
{
	GENERATED_BODY()

public:
	UMRAbility_Carve();

	/** AMRPlayerCharacter에서 박리 대상 몬스터를 설정 후 어빌리티 활성화 */
	void SetTargetMonster(AMRMonster* Monster);

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

	/** 박리 애니메이션 몽타주. BP에서 설정. */
	UPROPERTY(EditDefaultsOnly, Category = "Carve")
	TObjectPtr<UAnimMontage> CarveMontage;

private:
	UPROPERTY()
	TWeakObjectPtr<AMRMonster> TargetMonster;

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();
};
