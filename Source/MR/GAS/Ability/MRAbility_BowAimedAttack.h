// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRGameplayAbility.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "MRAbility_BowAimedAttack.generated.h"

class UAnimMontage;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

/**
 * 활 조준(Aimed) 공격 어빌리티.
 *
 * BowAttack과 구조적으로 동일하나 조준 상태에서만 사용하는
 * 별도 몽타주와 강화된 스태미나 비용/데미지 배율을 갖는다.
 *
 * BP 설정 필요 항목:
 *   - AimedFireMontage: 조준 발사 애니메이션 몽타주
 *   - DamageEffectClass: 히트 시 적용할 데미지 GE
 *   - StaminaCostEffectClass: 스태미나 소모 GE
 */
UCLASS(Blueprintable)
class MR_API UMRAbility_BowAimedAttack : public UMRGameplayAbility
{
	GENERATED_BODY()

public:
	UMRAbility_BowAimedAttack();

	/** 조준 발사 애니메이션 몽타주. BP에서 설정. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bow|AimedAttack")
	TObjectPtr<UAnimMontage> AimedFireMontage;

	/** 히트 시 적용할 데미지 GE. BP에서 설정. */
	UPROPERTY(EditDefaultsOnly, Category = "Bow|AimedAttack")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** 발사 시 소모할 스태미나 GE. BP에서 설정. */
	UPROPERTY(EditDefaultsOnly, Category = "Bow|AimedAttack")
	TSubclassOf<UGameplayEffect> StaminaCostEffectClass;

	/** 기본 스태미나 소모량 (조준 공격은 일반 공격보다 더 많이 소모) */
	UPROPERTY(EditDefaultsOnly, Category = "Bow|AimedAttack", meta = (ClampMin = "0.0"))
	float BaseStaminaCost = 25.f;

	/** AttackPower에 곱하는 데미지 배율. 조준 공격은 더 높은 배율 적용. */
	UPROPERTY(EditDefaultsOnly, Category = "Bow|AimedAttack", meta = (ClampMin = "0.1"))
	float MotionValue = 1.5f;

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

private:
	/** 스태미나를 즉시 소모한다. */
	void ApplyStaminaCost();

	/** 타겟 ASC에 데미지 GE를 적용한다. */
	void ApplyDamageToTarget(UAbilitySystemComponent* TargetASC);

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnHitEventReceived(FGameplayEventData Payload);

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> HitEventTask;
};
